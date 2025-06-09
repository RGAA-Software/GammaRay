//
// Created by RGAA on 20/02/2025.
//

#include "monitor_refresher.h"
#include "rd_context.h"
#include "tc_common_new/message_notifier.h"
#include "app/app_messages.h"
#include "tc_common_new/log.h"

#include <QApplication>
#include <QList>
#include <QScreen>
#include <QPainter>

namespace tc
{

    // Widget
    MonitorRefreshWidget::MonitorRefreshWidget(const std::shared_ptr<RdContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    void MonitorRefreshWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0,0,0,0)));
        //painter.setBrush(QBrush(QColor(10,10,10)));
        painter.drawRect(0, 0, this->width(), this->height());
    }

    // Refresher
    MonitorRefresher::MonitorRefresher(const std::shared_ptr<RdContext>& ctx, QWidget* parent) {
        context_ = ctx;

        QList<QWidget*> widgets;
        QList<QScreen*> screen_list = QGuiApplication::screens();
        for (const QScreen* screen : screen_list) {
            LOGI("screen name: {}, x: {}, y: {}", screen->name().toStdString(), screen->geometry().x(), screen->geometry().y());
            auto w = new MonitorRefreshWidget(context_, parent);
            int size = 10;
            w->setFixedSize(size, size);
            w->setGeometry(screen->geometry().x(), screen->geometry().y(), size, size);
            w->show();
            widgets.append(w);
        }

        msg_listener_ = context_->CreateMessageListener();
        msg_listener_->Listen<MsgTimer100>([=, this](const MsgTimer100& msg) {
            context_->PostUITask([=]() {
                for (QWidget* w : widgets) {
                    w->update();
                }
            });
        });
    }
}