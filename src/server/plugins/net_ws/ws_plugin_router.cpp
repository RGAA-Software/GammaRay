//
// Created by RGAA on 2024/3/5.
//

#include "ws_plugin_router.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "ws_plugin.h"

namespace tc
{

    void WsPluginRouter::OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnOpen(sess_ptr);
    }

    void WsPluginRouter::OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnClose(sess_ptr);
    }

    void WsPluginRouter::OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
        WsRouter::OnMessage(sess_ptr, data);
        auto plugin = Get<WsPlugin*>("plugin");
        auto msg = std::string(data.data(), data.size());
        plugin->CallbackClientEvent(plugin, msg);
    }

    void WsPluginRouter::OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPing(sess_ptr);
    }

    void WsPluginRouter::OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPong(sess_ptr);
    }

    void WsPluginRouter::PostBinaryMessage(const std::shared_ptr<Data> &data) {
        this->PostBinaryMessage(data->AsString());
    }

    void WsPluginRouter::PostBinaryMessage(const std::string &data) {
        if (session_ && session_->is_started()) {
            if (queued_message_count_ >= kMaxQueuedMessage) {
                LOGW("Too many queued message, discard the message in WsPluginRouter.");
                return;
            }
            queued_message_count_++;
            session_->async_send(data, [=, this](size_t byte_sent) {
                queued_message_count_--;
            });
        }
    }
}
