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

        const int kMaxCount = 1;
        for (int index = 0; index < kMaxCount; ++index) {
            /*ConnectedPair pair;
            pair.tag_ = new ConnectedInfoTag(nullptr);
            pair.panel_ = new ConnectedInfoPanel(nullptr);
            connected_panel_group_[index] = pair;*/

            ConnectedInfoSlidingWindow* sliding_window = new ConnectedInfoSlidingWindow(gr_ctx_);

            connected_info_panel_group_[index] = sliding_window;
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
       

    }

    void GrConnectedManager::AdjustPanelPosition() {
        auto screen_rect = QApplication::primaryScreen()->availableGeometry();
        int screen_width = screen_rect.width();
        int screen_height = screen_rect.height();

        for (auto& item: connected_info_panel_group_) {
            int panel_x = screen_width - item.second->width();
            int panel_y = screen_height - item.second->height() - 80 - item.first * item.second->height();
            item.second->move(panel_x, panel_y);
            int panel_rect_offset = item.second->GetRectOffset();

            item.second->show();
            
        }
    }
}