//
// Created by RGAA on 17/08/2024.
//

#include "float_sub_fps_panel.h"
#include "no_margin_layout.h"
#include "switch_button.h"
#include "background_widget.h"
#include "client/ct_settings.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "tc_common_new/log.h"
#include "single_selected_list.h"
#include <QLabel>

namespace tc
{

    SubFpsPanel::SubFpsPanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent) : BaseWidget(ctx, parent) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->setStyleSheet("background:#00000000;");
        int offset = 5;
        setFixedSize(210, 250);
        auto item_height = 35;
        auto border_spacing = 10;
        auto item_size = QSize(this->width() - 2*offset, item_height);
        auto root_layout = new NoMarginVLayout();
        root_layout->setContentsMargins(offset, offset, offset, offset);
        settings_ = Settings::Instance();

        fps_info_[EFps::k15Fps] = 15;
        fps_info_[EFps::k30Fps] = 30;
        fps_info_[EFps::k60Fps] = 60;
        fps_info_[EFps::k90Fps] = 90;
        fps_info_[EFps::k120Fps] = 120;
        fps_info_[EFps::k144Fps] = 144;

        listview_ = new SingleSelectedList(this);
        listview_->setFixedSize(QSize(this->width() - 2*offset, this->height() - 2*offset));
        listview_->UpdateItems({
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "15",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "30",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "60",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "90",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "120",
                   .icon_path_ = "",
            }),
            std::make_shared<SingleItem>(SingleItem {
                   .name_ = "144",
                   .icon_path_ = "",
            })
        });
        int target_index = GetFpsIndex(settings_->fps_);
        listview_->Select(target_index);
        root_layout->addWidget(listview_);
        setLayout(root_layout);
        UpdateStatus(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kImageScaleMode });
        listview_->SetOnItemClickListener([=, this](int idx, QWidget* w) {
            EFps efps = static_cast<EFps>(idx);
            int fps = 30;
            if (fps_info_.count(efps)) {
                fps = fps_info_[efps];
            }
            settings_->SetFps(fps);
            context_->SendAppMessage(MsgClientModifyFps{
                .fps_ = fps,
            });
            context_->SendAppMessage(MsgClientFloatControllerPanelUpdate{ .update_type_ = MsgClientFloatControllerPanelUpdate::EUpdate::kFps });
        });

    }

    void SubFpsPanel::paintEvent(QPaintEvent *event) {
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

    void SubFpsPanel::UpdateStatus(const MsgClientFloatControllerPanelUpdate& msg) {
        if (MsgClientFloatControllerPanelUpdate::EUpdate::kFps == msg.update_type_) {
            int fps = settings_->fps_;
            int list_index = GetFpsIndex(fps);
            listview_->Select(list_index);
        }
    }

    int SubFpsPanel::GetFpsIndex(int fps) {
        int res = 1;
        for (auto it = fps_info_.begin(); it != fps_info_.end(); ++it) {
            if (fps == it->second) {
                res = static_cast<int>(it->first);
                break;
            }
        }
        return res;
    }
}