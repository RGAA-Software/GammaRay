//
// Created by RGAA on 26/04/2025.
//

#include "stat_capture_info_item.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/tc_label.h"

namespace tc
{

    StatCaptureInfoItem::StatCaptureInfoItem(const std::shared_ptr<GrContext>& ctx, QWidget* parent) : TcBaseWidget(parent) {
        context_ = ctx;

        auto root_layout = new NoMarginVLayout();
        setLayout(root_layout);

        auto item_spacing = 5;
        //
        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(10);
            root_layout->addSpacing(10);
            root_layout->addLayout(layout);

            auto w = new TcLabel(this);
            w->setStyleSheet("font-size: 13px; font-weight:700; color: #444444;");
            lbl_target_name_ = w;
            w->setText("");
            layout->addWidget(w);
            layout->addStretch();
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_spacing);
            root_layout->addSpacing(item_spacing);
            root_layout->addLayout(layout);

            auto w = new TcLabel(this);
            w->setStyleSheet("font-size: 12px; color: #444444;");
            lbl_capture_size_ = w;
            w->setText("");
            layout->addWidget(w);
            layout->addStretch();
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_spacing);
            root_layout->addSpacing(item_spacing);
            root_layout->addLayout(layout);

            auto w = new TcLabel(this);
            w->setStyleSheet("font-size: 12px; color: #444444;");
            lbl_capture_fps_ = w;
            w->setText("");
            layout->addWidget(w);
            layout->addStretch();
        }

        {
            auto layout = new NoMarginHLayout();
            layout->addSpacing(item_spacing);
            root_layout->addSpacing(item_spacing);
            root_layout->addLayout(layout);

            auto w = new TcLabel(this);
            w->setStyleSheet("font-size: 12px; color: #444444;");
            lbl_encode_fps_ = w;
            w->setText("");
            layout->addWidget(w);
            layout->addStretch();
        }

        root_layout->addStretch();
    }

    void StatCaptureInfoItem::paintEvent(QPaintEvent *event) {
        TcBaseWidget::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen;
        if (mouse_enter_ || selected_) {
            pen.setColor(0x888888);
        }
        else {
            if (lbl_target_name_->text().isEmpty()) {
                pen.setColor(0xdddddd);
            } else {
                pen.setColor(0xbbbbbb);
            }
        }
        pen.setStyle(Qt::PenStyle::DashDotDotLine);
        painter.setPen(pen);
        painter.drawRoundedRect(QRect(1, 1, this->width()-2, this->height()-2), 5, 5);

    }

    std::string StatCaptureInfoItem::GetTargetName() {
        return lbl_target_name_->text().toStdString();
    }

    void StatCaptureInfoItem::UpdateInfo(const PtMsgWorkingCaptureInfo& info) {
        lbl_target_name_->setText(info.target_name().c_str());
        lbl_capture_size_->setText(std::format("Capture Size: {}x{}", info.capture_frame_width(), info.capture_frame_height()).c_str());
        lbl_capture_fps_->setText(std::format("Capture FPS: {}",info.capturing_fps()).c_str());
        lbl_encode_fps_->setText(std::format("Encoder: {}, FPS: {}", info.encoder_name(), info.encoding_fps()).c_str());
        update();
    }

    void StatCaptureInfoItem::ClearInfo() {
        lbl_target_name_->setText("");
        lbl_capture_size_->setText("");
        lbl_capture_fps_->setText("");
        lbl_encode_fps_->setText("");
        update();
    }

    void StatCaptureInfoItem::Select() {
        selected_ = true;
        update();
    }

    void StatCaptureInfoItem::Unselect() {
        selected_ = false;
        update();
    }

}