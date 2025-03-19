//
// Created by RGAA on 2023/8/19.
//

#include "stream_item_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include "app_color_theme.h"
#include "tc_common_new/uid_spacer.h"
#include "tc_qt_widget/tc_image_button.h"

namespace tc
{

    StreamItemWidget::StreamItemWidget(const StreamItem& item, int bg_color, QWidget* parent) : QWidget(parent) {
        this->item_ = item;
        this->bg_color_ = bg_color;
        this->setStyleSheet("background:#00000000;");
        if (icon_.isNull()) {
            icon_ = QPixmap::fromImage(QImage(":/resources/image/windows.svg"));
            icon_ = icon_.scaled(icon_.width() / 4.4, icon_.height() / 4.4, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        if (bg_pixmap_.isNull()) {
            bg_pixmap_ = QPixmap::fromImage(QImage(":/resources/image/test_cover.jpg"));
        }

        // connect button
        auto btn_conn = new QPushButton(this);
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
        btn_conn_ = btn_conn;
        btn_conn->setFixedSize(120, 30);
        btn_conn->setText(tr("Connect"));

        auto btn_option = new TcImageButton(":/resources/image/ic_vert_dots.svg", QSize(22, 22), this);
        btn_option->setFixedSize(30, 30);
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
            QRect info_rect(0, 0, width, 155);

            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            path.addRoundedRect(info_rect, radius_, radius_);
            QRect temp_rect(info_rect.left(), info_rect.top()+info_rect.height()/2, info_rect.width(), info_rect.height()/2);
            path.addRect(temp_rect);
            painter.fillPath(path,  QBrush(QColor(93, 201, 87)));
            path.closeSubpath();

            painter.setClipPath(path);
            painter.drawPixmap(info_rect, bg_pixmap_);

            painter.setBrush(QBrush(QColor(0xff, 0xff, 0xff, 0xee)));
            painter.drawPath(path);
            painter.restore();
        }

        int border_width = 2;
        {
            auto font = painter.font();
            font.setBold(true);
            font.setPointSize(15);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x333333)));
            std::string stream_name = item_.stream_name;
            painter.drawText(QRect(20, 0, this->width(), 50), Qt::AlignVCenter, stream_name.c_str());
        }

        QPen pen;
        if (enter_) {
            pen.setColor(QColor(AppColorTheme::kAppMenuItemBgHoverColor));
        } else {
            pen.setColor(QColor(0xffffff));
        }
        pen.setWidth(border_width);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(border_width/2, border_width/2, width() - border_width, height() - border_width, radius_-2, radius_-2);

        int margin = 20;
        painter.drawPixmap(QWidget::width() - icon_.width() - margin, 92, icon_);
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
        btn_conn_->setGeometry(20, y, btn_conn_->width(), btn_conn_->height());

        btn_option_->setGeometry(this->width() - btn_option_->width() - 5, y, btn_option_->width(), btn_option_->height());
    }

}