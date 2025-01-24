//
// Created by RGAA on 2023/8/16.
//

#include "stream_content.h"

#include "settings.h"
#include "tc_common_new/log.h"
#include "client_context.h"
#include "app_stream_list.h"
#include "widget_helper.h"
#include "create_stream_dialog.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>

namespace tc
{

    AddButton::AddButton(QWidget* parent) : QLabel(parent) {
        setFixedSize(50, 50);
        setStyleSheet("background:#00000000;");
        auto ps = new QGraphicsDropShadowEffect();
        ps->setBlurRadius(15);
        ps->setOffset(0, 0);
        ps->setColor(0x999999);
        this->setGraphicsEffect(ps);
    }

    void AddButton::paintEvent(QPaintEvent *) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::RenderHint::Antialiasing);
        int gap = 2;
        {
            QPen pen;
            QColor c;
            if (pressed_) {
                c = QColor(0x4fb4ff);
            } else if (enter_) {
                c = QColor(0x4fb4ff);
            } else {
                c = QColor(0x4fb4ff - 0x111111);
            }
            painter.setBrush(QBrush(c));
            pen.setColor(c);
            pen.setWidth(2);
            painter.setPen(pen);

            int w = QWidget::width() - 2 * gap;
            int h = QWidget::height() - 2 * gap;

            painter.drawRoundedRect(gap, gap, w, h, w / 2, h / 2);
        }

        {
            painter.setBrush(Qt::NoBrush);
            QPen pen(QColor(0xFFFFFF));
            pen.setWidth(2);
            painter.setPen(pen);

            int w = QWidget::width();
            int h = QWidget::height();
            int line_size = 16;
            int x_offset = (w - line_size) / 2;
            painter.drawLine(x_offset, h/2, x_offset + line_size, h / 2);

            int y_offset = (h - line_size) / 2;
            painter.drawLine(w/2, y_offset, w/2, y_offset + line_size);
        }
    }

    void AddButton::enterEvent(QEnterEvent *event) {
        enter_ = true;
        update();
    }

    void AddButton::leaveEvent(QEvent *event) {
        enter_ = false;
        update();
    }

    void AddButton::mousePressEvent(QMouseEvent *ev) {
        pressed_ = true;
        update();
    }

    void AddButton::mouseReleaseEvent(QMouseEvent *ev) {
        pressed_ = false;
        update();

        if (click_cbk_) {
            click_cbk_();
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

    StreamContent::StreamContent(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : AppContent(ctx, parent) {

        auto root_layout = new QVBoxLayout();
        WidgetHelper::ClearMargin(root_layout);

        auto stream_list = new AppStreamList(context_, this);
        stream_list_ = stream_list;
        root_layout->addWidget(stream_list);

        setLayout(root_layout);

        add_btn_ = new AddButton(this);
        add_btn_->SetOnClickCallback([=, this]() {
            CreateStreamDialog dialog(context_);
            dialog.exec();
        });

        empty_tip_ = new QLabel(this);
        int empty_size = 64;
        empty_tip_->resize(empty_size, empty_size);
        auto pixmap = QPixmap::fromImage(QImage(":/resources/image/empty.svg"));
        pixmap = pixmap.scaled(empty_size, empty_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        empty_tip_->setPixmap(pixmap);

        //
        stream_list_->LoadStreamItems();
    }

    StreamContent::~StreamContent() {

    }

    void StreamContent::OnContentShow() {
        AppContent::OnContentShow();
    }

    void StreamContent::OnContentHide() {
        AppContent::OnContentHide();
    }

    void StreamContent::resizeEvent(QResizeEvent *event) {
        int width = event->size().width();
        int height = event->size().height();
        int gap = 25;
        add_btn_->setGeometry(width - add_btn_->width() - gap, height - add_btn_->height() - gap, add_btn_->width(), add_btn_->height());
        empty_tip_->setGeometry((width - empty_tip_->width())/2, (height - empty_tip_->height())/2, empty_tip_->width(), empty_tip_->height());
    }

    void StreamContent::ShowEmptyTip() {
        empty_tip_->show();
    }

    void StreamContent::HideEmptyTip() {
        empty_tip_->hide();
    }

}