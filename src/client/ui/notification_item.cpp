//
// Created by RGAA on 17/07/2024.
//

#include"notification_item.h"
#include "no_margin_layout.h"
#include "client_context.h"
#include "tc_common_new/log.h"
#include "tc_common_new/num_formatter.h"
#include "tc_common_new/time_ext.h"
#include <QLabel>
#include <QGraphicsDropShadowEffect>

namespace tc
{

    NotificationItem::NotificationItem(const std::shared_ptr<ClientContext>& ctx, const std::string& nid, const std::string& icon_path, QWidget* parent) : QWidget(parent) {
        context_ = ctx;
        nid_ = nid;

        this->setObjectName("notification_item");
        this->setStyleSheet(R"(#notification_item {background-color:#00000000;}")");

        auto ps = new QGraphicsDropShadowEffect();
        ps->setBlurRadius(5);
        ps->setOffset(0, 0);
        ps->setColor(0xbbbbbb);
        this->setGraphicsEffect(ps);

        auto root_layout = new NoMarginHLayout();
        root_layout->addSpacing(30);
        root_layout->setAlignment(Qt::AlignVCenter);
        {
            auto lbl = new QLabel(this);
            icon_ = lbl;
            lbl->setFixedSize(45, 45);
            root_layout->addWidget(lbl);

            if (!icon_path.empty()) {
                QImage image;
                image.load(icon_path.c_str());
                icon_pixmap_ = QPixmap::fromImage(image);
                icon_pixmap_ = icon_pixmap_.scaled(lbl->width(), lbl->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                lbl->setPixmap(icon_pixmap_);
            }

        }
        {
            auto info_layout = new NoMarginVLayout();
            auto lbl = new QLabel(this);
            title_ = lbl;
            title_->setText("Title");
            title_->setFixedWidth(210);
            title_->setStyleSheet("background:transparent;font-size: 15px; font-weight:700;");
            info_layout->addSpacing(13);
            info_layout->addWidget(title_);

            {
                auto layout = new QHBoxLayout();

                transfer_info_ = new QLabel();
                transfer_info_->setText("0KB/s  0MB");
                layout->addWidget(transfer_info_);

                layout->addStretch();

                progress_info_ = new QLabel(this);
                progress_info_->setAlignment(Qt::AlignRight);
                progress_info_->setFixedWidth(80);
                progress_info_->setText("0/100");
                layout->addWidget(progress_info_);
                info_layout->addSpacing(4);
                info_layout->addLayout(layout);
            }

            progress_ = new QProgressBar();
            progress_->setMaximum(100);
            progress_->setValue(0);
            progress_->setFixedSize(210, 5);
            info_layout->addSpacing(1);
            info_layout->addWidget(progress_);

            {
                auto layout = new NoMarginHLayout();
                info_layout->addSpacing(5);
                info_layout->addLayout(layout);

                layout->addStretch();
                auto lbl = new QLabel();
                layout->addWidget(lbl);
                layout->addSpacing(0);
                lbl->setText(TimeExt::FormatTimestamp(TimeExt::GetCurrentTimestamp()).c_str());
            }

            info_layout->addStretch();
            root_layout->addSpacing(10);
            root_layout->addLayout(info_layout);
        }

        root_layout->addStretch();
        setLayout(root_layout);
    }

    void NotificationItem::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if (pressed_) {
            painter.setBrush(QColor(0xefefff));
        } else if (enter_) {
            painter.setBrush(QColor(0xf2f2f2ff));
        } else {
            painter.setBrush(QColor(0xffffff));
        }
        painter.setPen(Qt::NoPen);
        int offset = 15;
        float radius = 7;
        painter.drawRoundedRect(offset-4, 0, this->width()-offset*2, this->height(), radius, radius);

        if (state_ == NotificationState::kNotificationSuccess) {
            painter.setBrush(QBrush(QColor(0x00bb00)));
            painter.drawRoundedRect(offset+5, 5, 4, 4, 2, 2);
            painter.drawRoundedRect(offset+15, 5, 4, 4, 2, 2);
            painter.drawRoundedRect(offset+25, 5, 4, 4, 2, 2);
        } else if (state_ == NotificationState::kNotificationFailed) {
            painter.setBrush(QBrush(QColor(0xbb0000)));
            painter.drawRoundedRect(offset+5, 5, 4, 4, 2, 2);
            painter.drawRoundedRect(offset+15, 5, 4, 4, 2, 2);
            painter.drawRoundedRect(offset+25, 5, 4, 4, 2, 2);
        }
    }

    void NotificationItem::enterEvent(QEnterEvent *event) {
        enter_ = true;
        repaint();
    }

    void NotificationItem::leaveEvent(QEvent *event) {
        enter_ = false;
        repaint();
    }

    void NotificationItem::mousePressEvent(QMouseEvent *event) {
        pressed_ = true;
        repaint();
    }

    void NotificationItem::mouseReleaseEvent(QMouseEvent *event) {
        pressed_ = false;
        repaint();
    }

    void NotificationItem::UpdateTitle(const std::string& title) {
        context_->PostUITask([=, this]() {
            //title_->setText(title.c_str());
            QFontMetrics fontWidth(title_->font());
            QString elideNote = fontWidth.elidedText(title.c_str(), Qt::ElideRight, 210);
            title_->setText(elideNote);
            title_->setToolTip(title.c_str());
        });
    }

    void NotificationItem::UpdateProgress(int progress) {
        context_->PostUITask([=, this]() {
            if (last_progress_update_time_ == 0) {
                last_progress_update_time_ = progress_update_time_;
            }
            auto diff_time = progress_update_time_ - last_progress_update_time_;
            if (diff_time > 1000) {
                auto diff_data_size = progress_data_size_ - last_progress_data_size_;
                auto diff_time_in_seconds = diff_time / 1000.0f;
                auto speed = (uint64_t)(diff_data_size / diff_time_in_seconds);
                last_speed_str_ = NumFormatter::FormatStorageSize(speed);
                last_progress_data_size_ = progress_data_size_;
                last_progress_update_time_ = progress_update_time_;
                transfer_info_->setText(std::format("{}/s  {}", last_speed_str_, NumFormatter::FormatStorageSize(progress_data_size_)).c_str());
            }

            if (progress == 100) {
                transfer_info_->setText(std::format("{}/s  {}", last_speed_str_.empty() ? "0" : last_speed_str_, NumFormatter::FormatStorageSize(progress_data_size_)).c_str());
                progress_over_ = true;
            }

            progress_->setValue(progress);
            progress_info_->setText(std::format("{}/100", progress).c_str());
        });
    }

    void NotificationItem::UpdateProgressUpdateTime(uint64_t time) {
        progress_update_time_ = time;
    }

    void NotificationItem::UpdateProgressDataSize(uint64_t size) {
        progress_data_size_ = size;
    }

    void NotificationItem::SetState(const NotificationState& state) {
        state_ = state;
        repaint();
        if (state == NotificationState::kNotificationFailed) {
            progress_over_ = true;
        }
    }

    bool NotificationItem::IsProgressOver() {
        return progress_over_;
    }

    void NotificationItem::SetBelongToItem(QListWidgetItem* item) {
        belong_to_ = item;
    }

    QListWidgetItem* NotificationItem::BelongToItem() {
        return belong_to_;
    }

}