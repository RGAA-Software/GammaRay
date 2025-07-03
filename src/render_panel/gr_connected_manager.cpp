#include "gr_connected_manager.h"
#include <qapplication.h>
#include "gr_context.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "gr_app_messages.h"
#include "tc_render_panel_message.pb.h"
#include "tc_common_new/client_id_extractor.h"
#include "devices/connected_info_panel.h"
#include "devices/connected_info_tag.h"
#include "devices/connected_info_sliding_window.h"

namespace tc { 
	GrConnectedManager::GrConnectedManager(const std::shared_ptr<GrContext>& ctx) : gr_ctx_(ctx) {
		if (!gr_ctx_) {
			LOGE("gr_ctx_ is nullptr.");
			return;
		}

        const int kMaxCount = 2;
        for (int index = 0; index < kMaxCount; ++index) {
            ConnectedInfoSlidingWindow* sliding_window = new ConnectedInfoSlidingWindow(gr_ctx_);
            connected_info_panel_group_[index] = sliding_window;
        }

        RegisterMessageListener();
        AdjustPanelPosition();
	}

    void GrConnectedManager::RegisterMessageListener() {
        msg_listener_ = gr_ctx_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgUpdateConnectedClientsInfo>([=, this](const MsgUpdateConnectedClientsInfo& msg) {

            if (!gr_ctx_) {
                LOGE("gr_ctx_ is nullptr.");
                return;
            }

            gr_ctx_->PostUITask([=, this]() { 
                int client_size = msg.clients_info_.size();
                if (0 == client_size) {
                    HideAllPanels();
                    return;
                }

                for (int index = 0; index < client_size; ++index) {
                    auto client_info = msg.clients_info_[index];
                    if (connected_info_panel_group_.count(index) > 0) {
                        connected_info_panel_group_[index]->show();
                        auto client_id = ExtractClientId(client_info->device_id());
                        connected_info_panel_group_[index]->UpdateInfo(QString::fromStdString(client_id), QString::fromStdString(client_info->device_name()));
                    }
                }

                int group_index = -1;
                for (auto& item : connected_info_panel_group_) {
                    ++group_index;
                    if (group_index < client_size) {
                        continue;
                    }
                    item.second->hide();
                }
            });
        });
    }

    void GrConnectedManager::TestShowPanel() {
        // test
    }

    void GrConnectedManager::AdjustPanelPosition() {
        auto screen_rect = QApplication::primaryScreen()->availableGeometry();
        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();

        for (auto& item: connected_info_panel_group_) {
            int panel_x = screen_width - item.second->width();
            int panel_y = screen_height - item.second->height() - 80 - item.first * item.second->height() * 1.2;
            item.second->move(panel_x, panel_y);
            item.second->hide();        
        }
    }

    void GrConnectedManager::HideAllPanels() {
        for (auto& item : connected_info_panel_group_) {
            item.second->hide();
        }
    }

    void GrConnectedManager::ShowAllPanels() {
        for (auto& item : connected_info_panel_group_) {
            item.second->show();
        }
    }
}