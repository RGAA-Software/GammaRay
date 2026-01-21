//
// Created by RGAA on 2023/8/19.
//

#include "stream_item_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QTimer>

#include "tc_spvr_client/spvr_stream.h"
#include "tc_common_new/uid_spacer.h"
#include "tc_qt_widget/tc_image_button.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "tc_qt_widget/tc_pushbutton.h"
#include "tc_qt_widget/tc_label.h"
#include "client/ct_stream_item_net_type.h"

namespace tc
{

    StreamItemWidget::StreamItemWidget(const std::shared_ptr<spvr::SpvrStream>& item, int bg_color, QWidget* parent) : QWidget(parent) {
        this->item_ = item;
        this->bg_color_ = bg_color;
        this->setStyleSheet("background:#00000000;");
        if (icon_.isNull()) {
            if (item->HasRelayInfo()) {
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

        lbl_connecting_ = new TcLabel(this);
        lbl_connecting_->setFixedSize(100, 25);
        lbl_connecting_->SetTextId("id_connecting");
        lbl_connecting_->hide();

        connect(btn_conn, &QPushButton::clicked, this, [=, this]() {
            if (conn_listener_) {
                conn_listener_();
            }
        });

        auto btn_option = new TcImageButton(":/resources/image/ic_vert_dots.svg", QSize(22, 22), this);
        btn_option->SetColor(0, 0xf6f6f6, 0xeeeeee);
        btn_option->SetRoundRadius(15);
        btn_option->setFixedSize(25, 25);
        btn_option_ = btn_option;
        btn_option->SetOnImageButtonClicked([=, this]() {
            if (menu_listener_) {
                menu_listener_();
            }
        });

        // work mode
        work_mode_ = new TcLabel(this);
//        if (item->network_type_ == kStreamItemNtTypeWebSocket) {
//            // direct
//            work_mode_->SetTextId("id_direct");
//            work_mode_->setStyleSheet("font-size: 13px; font-weight: 700; color: #3e6682;");
//        }
//        else {
//            // relay
//            work_mode_->SetTextId("id_relay");
//            work_mode_->setStyleSheet("font-size: 13px; font-weight: 700; color: #438761;");
//        }
    }

    StreamItemWidget::~StreamItemWidget() {

    }

    void StreamItemWidget::ShowConnecting() {
        lbl_connecting_->show();
        QTimer::singleShot(2000, [this]() {
            lbl_connecting_->hide();
        });
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
        // stream name
        {
            QFont font(tcFontMgr()->font_name_);
            font.setBold(true);
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setPointSize(13);
            painter.setFont(font);
            //painter.setPen(QPen(QColor(0x555555)));
            painter.setPen(QPen(QColor(0x2979ff)));
            auto stream_name = item_->stream_name_;
            if (item_->HasRelayInfo()) {
                stream_name = tc::SpaceId(item_->remote_device_id_);
            }
            else {
                stream_name = item_->stream_host_;
            }
            painter.drawText(QRect(15, 0, this->width(), 40), Qt::AlignVCenter, stream_name.c_str());
        }

        int y_offset = 32;
        if (!item_->stream_name_.empty() && item_->stream_name_ != item_->stream_host_) {
            QFont font(tcFontMgr()->font_name_);
            font.setBold(true);
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setPointSize(10);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x77777777)));
            painter.drawText(QRect(15, y_offset, this->width(), 20), Qt::AlignVCenter, item_->stream_name_.c_str());
            y_offset += 20;
        }

        // desktop name
        {
            QFont font(tcFontMgr()->font_name_);
            font.setBold(false);
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setPointSize(10);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x77777777)));
            auto desktop_name = item_->desktop_name_;
            if (item_->HasRelayInfo()) {
                desktop_name = tc::SpaceId(desktop_name);
            }
            painter.drawText(QRect(15, y_offset, this->width(), 20), Qt::AlignVCenter, desktop_name.c_str());
            y_offset += 20;
        }

        // os version
        {
            QFont font(tcFontMgr()->font_name_);
            font.setBold(false);
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setPointSize(10);
            painter.setFont(font);
            painter.setPen(QPen(QColor(0x77777777)));
            auto os_version = QString::fromStdString(item_->os_version_);
            os_version = os_version.toUpper();
            painter.drawText(QRect(15, y_offset, this->width(), 20), Qt::AlignVCenter, os_version);
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

        int margin_top = 30;
        painter.drawPixmap(QWidget::width() - icon_.width() - 20, margin_top, icon_);

        int indicator_width = 10;
        int indicator_height = 8;
        int indicator_radius = 4;
        int margin_right = 50;
        for (int i = 0; i < 3; i++) {
            if (direct_connected_ && i == 0) {
                painter.setBrush(QBrush(0x00ff00));
            }
            else if (relay_connected_ && i == 1) {
                painter.setBrush(QBrush(0x00ff00));
            }
            else if (spvr_connected_ && i == 2) {
                painter.setBrush(QBrush(0x00ff00));
            }
            else {
                painter.setBrush(QBrush(0xbbbbbb));
            }
            painter.setPen(Qt::NoPen);
            auto x = this->width() - margin_right + indicator_width * i;
            auto y = 10;
            painter.drawRoundedRect(x, y, indicator_width, indicator_height, 0, 0);
            if (i == 1 || i == 2) {
                QPen pen(QColor(0x555555));
                pen.setWidth(1);
                painter.setPen(pen);
                painter.drawLine(x, y+2 , x, y + indicator_height - 2);
            }
        }

        {
            QPen pen(QColor(0x555555));
            pen.setWidth(1);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(this->width() - margin_right, 10, indicator_width * 3, indicator_height, indicator_radius, indicator_radius);
        }
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
        lbl_connecting_->setGeometry(15* 2 + btn_conn_->width(), y, lbl_connecting_->width(), lbl_connecting_->height());
        btn_option_->setGeometry(this->width() - btn_option_->width() - 13, y, btn_option_->width(), btn_option_->height());
        work_mode_->setGeometry(15 + btn_conn_->width() + 15, y, btn_conn_->width(), btn_conn_->height());
    }

    void StreamItemWidget::SetOnConnectListener(OnConnectListener&& listener) {
        conn_listener_ = listener;
    }

    void StreamItemWidget::SetOnMenuListener(OnMenuListener&& listener) {
        menu_listener_ = listener;
    }

    void StreamItemWidget::SetDirectConnectedState(bool connected) {
        direct_connected_ = connected;
    }

    void StreamItemWidget::SetRelayConnectedState(bool connected) {
        relay_connected_ = connected;
    }

    void StreamItemWidget::SetSpvrConnectedState(bool connected) {
        spvr_connected_ = connected;
    }

    void StreamItemWidget::Update() {
        this->update();
    }

    std::string StreamItemWidget::GetStreamId() {
        return item_->stream_id_;
    }

}