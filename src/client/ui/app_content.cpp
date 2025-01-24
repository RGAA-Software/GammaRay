//
// Created by RGAA on 2023/8/16.
//

#include "app_content.h"

#include "settings.h"

#include <QPainter>
#include <QPen>
#include <QBrush>

namespace tc
{

    AppContent::AppContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : RoundRectWidget(0xFFFFFF, 10, parent) {
        context_ = ctx;
        settings_ = Settings::Instance();
    }

    AppContent::~AppContent() {

    }

    void AppContent::paintEvent(QPaintEvent *event) {
        RoundRectWidget::paintEvent(event);

//        QPainter painter(this);
//        painter.setPen(Qt::NoPen);
//        painter.setBrush(QBrush(QColor(0xEAF7FF)));
//        painter.drawRect(0, 0, QWidget::width(), QWidget::height());
    }

    void AppContent::OnContentShow() {

    }

    void AppContent::OnContentHide() {

    }

}