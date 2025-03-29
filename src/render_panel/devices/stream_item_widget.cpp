//
// Created by RGAA on 2023/8/19.
//

#include "stream_item_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include "tc_common_new/uid_spacer.h"
#include "tc_qt_widget/tc_image_button.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "tc_qt_widget/tc_pushbutton.h"

namespace tc
{

    StreamItemWidget::StreamItemWidget(const StreamItem& item, int bg_color, QWidget* parent) : QWidget(parent) {
        this->item_ = item;
        this->bg_color_ = bg_color;
        this->setStyleSheet("background:#00000000;");
        if (icon_.isNull()) {
            if (item.IsRelay()) {
                icon_ = QPixmap::fromImage(QImage(":/resources/image/ic_windows_relay.svg"));
            } else {
                icon_ = QPixmap::fromImage(QImage(":/resources/image/ic_windows_direct.svg"));
            }
            icon_ = icon_.scaled(icon_.width() / 6.2, icon_.height() / 6.2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (bg_pixmap_.isNull()) {
            bg_pixmap_ = QPixmap::fromImage(QImage(":/resources/image/test_cover.png"));
            bg_pixmap_ = bg_pixmap_.scaled(230, bg_pixmap_.height()*0.65, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // connect button
        auto btn_conn = new TcPushButton(this);
        btn_conn_ = btn_conn;
        // 在ListView中，单独设置一下
        btn_conn->setStyleSheet(R"(
            QPushButton {
                background-color:#2979ff;
                color: white;
            }
            QPushButton:hover{
                background-color:#2059ee;
            }
            QPushButton:pressed{
                background-color:#1549dd;
            }
        )");
        btn_conn_->setFixedSize(70, 25);
        btn_conn_->SetTextId("id_connect");

        auto btn_option = new TcImageButton(":/resources/image/ic_vert_dots.svg", QSize(22, 22), this);
        btn_option->SetColor(0, 0xf6f6f6, 0xeeeeee);
        btn_option->SetRoundRadius(15);
        btn_option->setFixedSize(25, 25);
        btn_option_ = btn_option;
    }

    StreamItemWidget::~StreamItemWidget() {

    }

    void StreamItemWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing, true);
        painter.setRenderHints(QPainter::TextAntialiasing, true);
        painter.setRenderHints(QPainter::SmoothPixmapTransform, true);
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(0xffffff));
            painter.drawRoundedRect(0, 0, width(), height(), radius_, radius_);
        }

        {
            painter.save();
            int width = this->width();
            QRect info_rect(0, 0, width, bg_pixmap_.height() + 7);

            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(info_rect, radius_, radius_);
            QRect temp_rect(info_rect.left(), info_rect.top()+info_rect.height()/2, info_rect.width(), info_rect.height()/2);
            path.addRect(temp_rect);
            painter.fillPath(path,  QBrush(QColor(0xff, 0xff, 0xff)));
            path.closeSubpath();

            painter.setClipPath(path);
            painter.drawPixmap(info_rect, bg_pixmap_);

            painter.setBrush(QBrush(QColor(0xff, 0xff, 0xff, 0xdd)));
            painter.drawPath(path);
            painter.restore();
        }

        int border_width = 2;
        {
            QFont font(tcFontMgr()->font_name_);
            font.setBold(true);
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setPointSize(13);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x555555)));
            auto stream_name = item_.stream_name_;
            if (item_.IsRelay()) {
                stream_name = tc::SpaceId(stream_name);
            }
            painter.drawText(QRect(15, 0, this->width(), 40), Qt::AlignVCenter, stream_name.c_str());
        }

        QPen pen;
        if (enter_) {
            pen.setColor(QColor(0x2979ff));
        } else {
            pen.setColor(QColor(0xffffff));
        }
        pen.setWidth(border_width);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(border_width/2, border_width/2, width() - border_width, height() - border_width, radius_-2, radius_-2);

        int margin = 20;
        painter.drawPixmap(QWidget::width() - icon_.width() - margin, 10, icon_);
    }

    void StreamItemWidget::enterEvent(QEnterEvent *event) {
        enter_ = true;
        update();
    }

    void StreamItemWidget::leaveEvent(QEvent *event) {
        enter_ = false;
        update();
    }

    void StreamItemWidget::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event);
        auto y = this->height() - 35;
        btn_conn_->setGeometry(15, y, btn_conn_->width(), btn_conn_->height());
        btn_option_->setGeometry(this->width() - btn_option_->width() - 13, y, btn_option_->width(), btn_option_->height());
    }

}