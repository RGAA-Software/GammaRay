//
// Created by RGAA on 20/02/2025.
//

#include "monitor_refresher.h"
#include "render_panel/gr_context.h"
#include "render_panel/ui/monitor_refresher.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "render_panel/gr_app_messages.h"

#include <QApplication>
#include <QList>
#include <QScreen>
#include <QPainter>

namespace tc
{

    // Widget
    MonitorRefreshWidget::MonitorRefreshWidget(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    void MonitorRefreshWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(color_value_, color_value_/2, color_value_/3,5)));
        painter.drawRect(0, 0, this->width(), this->height());
        ++color_value_;
        color_value_ %= 255;
    }

    // Refresher
    MonitorRefresher::MonitorRefresher(const std::shared_ptr<GrContext>& ctx, QWidget* parent) {
        context_ = ctx;
        QList<QScreen*> screen_list = QGuiApplication::screens();
        for (const QScreen* screen : screen_list) {
            LOGI("screen name: {}, x: {}, y: {}", screen->name().toStdString(), screen->geometry().x(), screen->geometry().y());
            auto w = new MonitorRefreshWidget(context_, parent);
            int size = 5;
            w->setFixedSize(size, size);
            w->setGeometry(screen->geometry().x(), screen->geometry().y(), size, size);
            w->show();
            widgets_.append(w);
        }

        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer100>([=, this](const MsgGrTimer100& msg) {
            Refresh();
        });
    }

    void MonitorRefresher::Refresh() {
        if (!context_) {
            return;
        }
        context_->PostUITask([=]() {
            for (QWidget* w : widgets_) {
                w->update();
            }
        });
    }
}