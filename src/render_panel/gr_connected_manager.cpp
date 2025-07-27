#include "gr_connected_manager.h"
#include <qapplication.h>
#include <Windows.h>
#include "gr_context.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "gr_app_messages.h"
#include "tc_render_panel_message.pb.h"
#include "tc_common_new/client_id_extractor.h"
#include "devices/connected_info_panel.h"
#include "devices/connected_info_tag.h"
#include "devices/connected_info_sliding_window.h"
#include "gr_settings.h"

namespace tc { 
	GrConnectedManager::GrConnectedManager(const std::shared_ptr<GrContext>& ctx) : gr_ctx_(ctx) {
		if (!gr_ctx_) {
			LOGE("gr_ctx_ is nullptr.");
			return;
		}

        CreatePanel();
        RegisterMessageListener();
        InitPanel();
	}

    bool GrConnectedManager::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_DISPLAYCHANGE) {
            if (gr_ctx_) {
                gr_ctx_->PostUIDelayTask([=, this]() {
                    //LOGI("nativeEventFilter WM_DISPLAYCHANGE");
                    AdjustPanelPosition();
                }, 4000);
            }
        }
        return false;
    }

    void GrConnectedManager::RegisterMessageListener() {
        msg_listener_ = gr_ctx_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgUpdateConnectedClientsInfo>([=, this](const MsgUpdateConnectedClientsInfo& msg) {

            if (!gr_ctx_) {
                LOGE("gr_ctx_ is nullptr.");
                return;
            }

            client_connected_count_ = msg.clients_info_.size();

            gr_ctx_->PostUITask([=, this]() { 
                int client_size = msg.clients_info_.size();
                if (0 == client_size) {
                    HideAllPanels();
                    return;
                }
                //LOGI("MsgUpdateConnectedClientsInfo, client_size: {} ", client_size);
                for (int index = 0; index < client_size; ++index) {
                    auto client_info = msg.clients_info_[index];
                    if (connected_info_panel_group_.count(index) > 0) {
                        connected_info_panel_group_[index]->show();
                        const std::string old_stream_id =  connected_info_panel_group_[index]->GetStreamId();
                        connected_info_panel_group_[index]->UpdateInfo(client_info);
                        if (old_stream_id != client_info->stream_id()) {
                            connected_info_panel_group_[index]->Expand();
                        }
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

        msg_listener_->Listen<MsgOneClientDisconnect>([=, this](const MsgOneClientDisconnect& msg) {

            if (!gr_ctx_) {
                LOGE("gr_ctx_ is nullptr.");
                return;
            }

            gr_ctx_->PostUIDelayTask([=, this]() {
                auto settings = GrSettings::Instance();
                if (0 == client_connected_count_ && settings->IsDisconnectAutoLockScreenEnabled()) {
                    LockWorkStation();
                }
            }, 6000);
        });
    }

    void GrConnectedManager::TestShowPanel() {
        // test
    }

    void GrConnectedManager::AdjustPanelPosition() {
        auto primary_screen = QApplication::primaryScreen();
        if (!primary_screen) {
            return;
        }
        auto screen_rect = primary_screen->availableGeometry();
        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();
        int index = 0;
        for (auto& item: connected_info_panel_group_) {
            int panel_x = screen_width - item.second->width();
            int panel_y = screen_height - item.second->height() - 8 - item.first * item.second->height() * 1.1;
            item.second->move(panel_x, panel_y);
            //LOGI("index: {}, panel_x: {}, panel_y: {}", index, panel_x, panel_y);
            ++index;
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

    void GrConnectedManager::InitPanel() {
        AdjustPanelPosition();
    }

    void GrConnectedManager::CreatePanel() {
        for (auto& item : connected_info_panel_group_) {
            delete item.second;
        }
        connected_info_panel_group_.clear();
        const int kMaxCount = 2;
        for (int index = 0; index < kMaxCount; ++index) {
            ConnectedInfoSlidingWindow* sliding_window = new ConnectedInfoSlidingWindow(gr_ctx_);
            connected_info_panel_group_[index] = sliding_window;
            sliding_window->hide();
        }
    }
}