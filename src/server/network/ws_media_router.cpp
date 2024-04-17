//
// Created by hy on 2024/3/5.
//

#include "ws_media_router.h"
#include "tc_common_new/data.h"
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
        session_->async_send(data->CStr(), data->Size());
    }

    void WsMediaRouter::PostBinaryMessage(const std::string &data) {
        session_->async_send(data);
    }
}
