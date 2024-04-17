//
// Created by hy on 2024/3/5.
//

#include "ws_ipc_router.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"
#include "message_processor.h"
#include "tc_capture_new/capture_message.h"
#include "app.h"

namespace tc
{

    void WsIpcRouter::OnOpen(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnOpen(sess_ptr);
    }

    void WsIpcRouter::OnClose(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnClose(sess_ptr);
    }

    void WsIpcRouter::OnMessage(std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
        WsRouter::OnMessage(sess_ptr, data);
        auto base_msg = (CaptureBaseMessage*)data.data();
        auto app = Get<std::shared_ptr<Application>>("app");
        if (base_msg->type_ == kCaptureVideoFrame) {
            auto msg = std::make_shared<CaptureVideoFrame>();
            if (data.size() != sizeof(CaptureVideoFrame)) {
                LOGE("Error size of ipc video frame, data size: {}, CaptureVideoFrame size: {}", data.size(), sizeof(CaptureVideoFrame));
                return;
            }
            memcpy(msg.get(), data.data(), data.size());
            app->OnIpcVideoFrame(msg);
        }
    }

    void WsIpcRouter::OnPing(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPing(sess_ptr);
    }

    void WsIpcRouter::OnPong(std::shared_ptr<asio2::http_session> &sess_ptr) {
        WsRouter::OnPong(sess_ptr);
    }

    void WsIpcRouter::PostBinaryMessage(const std::shared_ptr<Data> &data) {
        session_->async_send(data->CStr(), data->Size());
    }

    void WsIpcRouter::PostBinaryMessage(const std::string &data) {
        session_->async_send(data);
    }
}
