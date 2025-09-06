//
// Created by RGAA on 17/08/2024.
//

#include "float_3rd_scale_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "client/ct_settings.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "single_selected_list.h"
#include "tc_label.h"
#include <memory>

namespace tc
{

    ThirdScalePanel::ThirdScalePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        setFixedSize(210, 140);
        auto root_layout = new NoMarginVLayout();
        int offset = 5;
        root_layout->setContentsMargins(offset, offset, offset, offset);

        settings_ = Settings::Instance();

        listview_ = new SingleSelectedList(this);
        listview_->setFixedSize(QSize(this->width() - 2*offset, this->height() - 2*offset));
        listview_->UpdateItems({
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = tcTr("id_keep_aspect_ratio"),
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = tcTr("id_full_window"),
                   .icon_path_ = "",
            }),
            // std::make_shared<SingleItem>(SingleItem {
            //        .name_ = "Original Size",
            //        .icon_path_ = "",
            // }),
        });
        root_layout->addWidget(listview_);
        setLayout(root_layout);
        UpdateStatus(MsgClientFloatControllerPanelUpdate{
            .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kImageScaleMode
        });

        listview_->SetOnItemClickListener([=, this](int idx, QWidget* w) {
            ScaleMode mode = ScaleMode::kFullWindow;
            if (idx == 0) { 
                mode = ScaleMode::kKeepAspectRatio;
            } else if (idx == 1) { 
                mode = ScaleMode::kFullWindow; 
            } else if (idx == 2) { 
                mode = ScaleMode::kOriginSize; 
            }
            UpdateScaleMode(mode);
            MsgClientSwitchScaleMode scale_mode_msg{.mode_ = mode};
            context_->SendAppMessage(scale_mode_msg);
            context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{
                .update_type_ = MsgClientFloatControllerPanelUpdate ::EUpdate::kImageScaleMode
            });
        });
    }

    void ThirdScalePanel::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        QPen pen(0xaaaaaa);
        pen.setWidth(2);
        pen.setStyle(Qt::PenStyle::DotLine);
        painter.setPen(pen);

        painter.setBrush(QColor(0xffffff));
        int offset = 0;
        int radius = 2;
        painter.drawRoundedRect(offset, offset, this->width()-offset*2, this->height()-offset*2, radius, radius);
        BaseWidget::paintEvent(event);
    }

    void ThirdScalePanel::UpdateScaleMode(ScaleMode mode) {
        settings_->SetScaleMode(mode);
    }

    void ThirdScalePanel::UpdateStatus(const MsgClientFloatControllerPanelUpdate& msg) {
        if (MsgClientFloatControllerPanelUpdate::EUpdate::kImageScaleMode == msg.update_type_) {
            int target_index = 0;
            if (ScaleMode::kKeepAspectRatio == settings_->scale_mode_) {
                target_index = 0;
            }
            else if (ScaleMode::kFullWindow == settings_->scale_mode_) {
                target_index = 1;
            }
            else if (ScaleMode::kOriginSize == settings_->scale_mode_) {
                target_index = 2;
            }
            listview_->Select(target_index);
        }
    }
}