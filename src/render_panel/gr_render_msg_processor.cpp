//
// Created by RGAA on 26/07/2025.
//

#include "gr_render_msg_processor.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_application.h"
#include "render_panel/clipboard/panel_clipboard_manager.h"

namespace tc
{

    GrRenderMsgProcessor::GrRenderMsgProcessor(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
    }

    void GrRenderMsgProcessor::OnMessage(std::shared_ptr<tc::Message> msg) {
        // clipboard
        {
            auto clipboard_mgr = context_->GetApplication()->GetClipboardManager();
            clipboard_mgr->OnRemoteClipboardInfo(msg);
        }
    }

}