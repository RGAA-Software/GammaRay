//
// Created by RGAA on 26/04/2025.
//

#include "ct_stat_frame_info_item.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "tc_qt_widget/tc_label.h"

namespace tc
{

    CtStatFrameInfoItem::CtStatFrameInfoItem(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : TcBaseWidget(parent) {
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
            lbl_render_size_ = w;
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
            lbl_received_fps_ = w;
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
            lbl_render_capture_fps_ = w;
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
            lbl_render_capture_size_ = w;
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
            lbl_render_encoder_name_ = w;
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
            lbl_render_encode_fps_ = w;
            w->setText("");
            layout->addWidget(w);
            layout->addStretch();
        }

        root_layout->addStretch();
    }

    void CtStatFrameInfoItem::paintEvent(QPaintEvent *event) {
        TcBaseWidget::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPen pen;
        if (mouse_enter_ || selected_) {
            pen.setColor(0x2979ff);
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

    std::string CtStatFrameInfoItem::GetTargetName() {
        return lbl_target_name_->text().toStdString();
    }

    void CtStatFrameInfoItem::UpdateInfo(const CtStatItemInfo& info) {
        lbl_target_name_->setText(info.name_.c_str());
        lbl_render_size_->setText(std::format("Frame Size: {}x{}", info.frame_width_, info.frame_height_).c_str());
        lbl_received_fps_->setText(std::format("Received FPS: {}", info.received_fps_).c_str());
        lbl_render_capture_fps_->setText(std::format("Capture FPS: {}", info.render_capture_fps_).c_str());
        lbl_render_capture_size_->setText(std::format("Capture Size: {}x{}", info.render_capture_frame_width_, info.render_capture_frame_height_).c_str());
        lbl_render_encoder_name_->setText(std::format("Encoder Name: {}", info.render_encoder_name_).c_str());
        lbl_render_encode_fps_->setText(std::format("Encoder FPS: {}", info.render_encode_fps_).c_str());
        update();
    }

    void CtStatFrameInfoItem::ClearInfo() {
        lbl_target_name_->setText("");
        lbl_render_size_->setText("");
        lbl_received_fps_->setText("");
        update();
    }

    void CtStatFrameInfoItem::Select() {
        selected_ = true;
        update();
    }

    void CtStatFrameInfoItem::Unselect() {
        selected_ = false;
        update();
    }

}