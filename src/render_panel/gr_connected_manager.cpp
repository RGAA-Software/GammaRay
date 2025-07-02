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

namespace tc { 
	GrConnectedManager::GrConnectedManager(const std::shared_ptr<GrContext>& ctx) : gr_ctx_(ctx) {
		if (!gr_ctx_) {
			LOGE("gr_ctx_ is nullptr.");
			return;
		}

        const int kMaxCount = 2;
        for (int index = 0; index < kMaxCount; ++index) {
            ConnectedPair pair;
            pair.tag_ = new ConnectedInfoTag(nullptr);
            pair.panel_ = new ConnectedInfoPanel(nullptr);
            connected_panel_group_[index] = pair;
        }

        RegisterMessageListener();
        AdjustPanelPosition();
        //TestShowPanel();
	}

    void GrConnectedManager::RegisterMessageListener() {
        msg_listener_ = gr_ctx_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgUpdateConnectedClientsInfo>([=, this](const MsgUpdateConnectedClientsInfo& msg) {
            // to do
            msg.clients_info_;
        });
    }

    void GrConnectedManager::TestShowPanel() {
        // test
        ConnectedInfoPanel* connected_info_panel = new ConnectedInfoPanel(nullptr);
        connected_info_panel->show();

        // test
        ConnectedInfoTag* tag = new ConnectedInfoTag(nullptr);
        tag->show();
        auto screen_rect = QApplication::primaryScreen()->availableGeometry();

        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();

        int panel_x = screen_width - connected_info_panel->width();
        int panel_y = screen_height - connected_info_panel->height();

        connected_info_panel->move(panel_x, panel_y);
        int panel_rect_offset = connected_info_panel->GetRectOffset();

        tag->move(panel_x + panel_rect_offset - tag->width(), panel_y + panel_rect_offset);

    }

    void GrConnectedManager::AdjustPanelPosition() {
        auto screen_rect = QApplication::primaryScreen()->availableGeometry();
        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();

        for (auto& item: connected_panel_group_) {
            int panel_x = screen_width - item.second.panel_->width();
            int panel_y = screen_height - item.second.panel_->height() - 40 - item.first * item.second.panel_->height();
            item.second.panel_->move(panel_x, panel_y);
            int panel_rect_offset = item.second.panel_->GetRectOffset();
            item.second.tag_->move(panel_x + panel_rect_offset - item.second.tag_->width(), panel_y + panel_rect_offset);

            item.second.panel_->show();
            item.second.tag_->show();
        }
    }
}