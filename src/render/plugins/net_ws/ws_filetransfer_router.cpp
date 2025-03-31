//
// Created by RGAA on 2024/3/5.
//

#include "ws_filetransfer_router.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "tc_common_new/thread_util.h"
#include "ws_plugin.h"
#include "tc_message.pb.h"

namespace tc
{

    void WsFileTransferRouter::OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnOpen(sess_ptr);
        LOGI("FileTransfer OnOpen.");
    }

    void WsFileTransferRouter::OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnClose(sess_ptr);
        LOGI("FileTransfer OnClose.");
    }

    void WsFileTransferRouter::OnMessage(std::shared_ptr<asio2::http_session>& sess_ptr, int64_t socket_fd, std::string_view data) {
        WsRouter::OnMessage(sess_ptr, socket_fd, data);
        auto plugin = Get<WsPlugin*>("plugin");
        auto msg = std::string(data.data(), data.size());
        plugin->OnClientEventCame(true, socket_fd, NetPluginType::kWebSocket, msg);
    }

    void WsFileTransferRouter::OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPing(sess_ptr);
    }

    void WsFileTransferRouter::OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPong(sess_ptr);
    }

    void WsFileTransferRouter::PostBinaryMessage(const std::string &data) {
        if (!session_ || !session_->is_started()) {
            return;
        }
        session_->post_queued_event([=, this]() {
            auto tid = tc::GetCurrentThreadID();
            if (post_thread_id_ == 0) {
                post_thread_id_ = tid;
            }
            if (tid != post_thread_id_) {
                LOGI("OH NO! Post binary message in thread: {}, but the last thread is: {}", tid, post_thread_id_);
            }

            session_->ws_stream().binary(true);
            queuing_message_count_++;
            session_->async_send(data, [=, this](size_t byte_sent) {
                queuing_message_count_--;
            });
        });
    }

}
