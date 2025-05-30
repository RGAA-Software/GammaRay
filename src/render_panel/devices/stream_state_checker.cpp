//
// Created by RGAA on 24/05/2025.
//

#include "stream_state_checker.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/http_client.h"
#include "render_panel/gr_context.h"
#include "render_panel/gr_app_messages.h"
#include "render_panel/database/stream_item.h"
#include "tc_spvr_client/spvr_device_info.h"

namespace tc
{

    StreamStateChecker::StreamStateChecker(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
        msg_listener_ = context_->ObtainMessageListener();
        msg_listener_->Listen<MsgGrTimer5S>([=, this](const MsgGrTimer5S& msg) {
            if (!thread_) {
                return;
            }
            thread_->Post([this]() {
                this->CheckState();
            });
        });
    }

    void StreamStateChecker::Start() {
        thread_ = Thread::Make("stream_state_checker", 120);
        thread_->Poll();
    }

    void StreamStateChecker::Exit() {
        if (thread_) {
            thread_->Exit();
        }
    }

    void StreamStateChecker::UpdateCurrentStreamItems(const std::vector<std::shared_ptr<StreamItem>>& items) {
        items_ = items;
        thread_->Post([this]() {
            this->CheckState();
        });
    }

    void StreamStateChecker::SetOnCheckedCallback(OnStreamStateCheckedCallback&& cbk) {
        on_checked_cbk_ = cbk;
    }

    void StreamStateChecker::CheckState() {
        for (auto& item : items_) {
            bool online = false;
            if (item->IsRelay()) {
                // to check in server
                auto device_info = context_->GetRelayServerSideDeviceInfo(item->remote_device_id_, false);
                if (device_info && device_info->IsValid()) {
                    online = true;
                }
            }
            else {
                // host & port mode
                // /api/ping
                auto client = HttpClient::MakeSSL(item->stream_host_, item->stream_port_, "/api/ping", 300);
                auto res = client->Request();
                if (res.status == 200) {
                    online = true;
                }
                else {
                    online = false;
                }
            }

            item->online_ = online;
        }

        if (on_checked_cbk_) {
            on_checked_cbk_(items_);
        }
    }

}
