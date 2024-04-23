//
// Created by RGAA on 2024/3/5.
//

#include "ws_media_router.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "message_processor.h"

namespace tc
{

    void WsMediaRouter::OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnOpen(sess_ptr);
    }

    void WsMediaRouter::OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnClose(sess_ptr);
    }

    void WsMediaRouter::OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
        WsRouter::OnMessage(sess_ptr, data);
        Get<std::shared_ptr<MessageProcessor>>("proc")->HandleMessage(data);
    }

    void WsMediaRouter::OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPing(sess_ptr);
    }

    void WsMediaRouter::OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPong(sess_ptr);
    }

    void WsMediaRouter::PostBinaryMessage(const std::shared_ptr<Data> &data) {
        this->PostBinaryMessage(data->AsString());
    }

    void WsMediaRouter::PostBinaryMessage(const std::string &data) {
        if (queued_message_count_ >= kMaxQueuedMessage) {
            LOGW("Too many queued message, discard the message in WsMediaRouter.");
            return;
        }
        queued_message_count_++;
        LOGI("current queued message count: {}", queued_message_count_);
        session_->async_send(data, [=, this](size_t byte_sent) {
            queued_message_count_--;
        });
    }
}
