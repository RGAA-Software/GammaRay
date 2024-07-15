//
// Created by hy on 8/07/2024.
//

#include "file_transfer.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "tc_common_new/thread.h"

namespace tc
{

    std::shared_ptr<FileTransferChannel> FileTransferChannel::Make(const std::shared_ptr<GrContext> &ctx) {
        return std::make_shared<FileTransferChannel>(ctx);
    }

    FileTransferChannel::FileTransferChannel(const std::shared_ptr<GrContext> &ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
        thread_ = Thread::Make("file transfer sender", 8192);
        thread_->Poll();
    }

    void FileTransferChannel::Start() {
        server_ = std::make_shared<asio2::ws_server>();
        server_->bind_accept([=, this](std::shared_ptr<asio2::ws_session> &session_ptr) {
            // accept callback maybe has error like "Too many open files", etc...
            if (!asio2::get_last_error()) {
                session_ = session_ptr;
                session_ptr->ws_stream().binary(true);

                // how to set custom websocket response data :
                // the decorator is just a callback function, when the upgrade response is send,
                // this callback will be called.
                session_ptr->ws_stream().set_option(
                        websocket::stream_base::decorator([session_ptr](websocket::response_type &rep) {
                            // @see /asio2/example/websocket/client/websocket_client.cpp
                            const websocket::request_type &req = session_ptr->get_upgrade_request();
                            auto it = req.find(http::field::authorization);
                            if (it != req.end())
                                rep.set(http::field::authentication_results, "200 OK");
                            else
                                rep.set(http::field::authentication_results, "401 unauthorized");
                        }));
            } else {
                printf("error occurred when calling the accept function : %d %s\n",
                       asio2::get_last_error_val(), asio2::get_last_error_msg().data());
            }
        }).bind_recv([&](auto &session_ptr, std::string_view data) {
            ParseMessage(data);
        }).bind_connect([](auto &session_ptr) {

        }).bind_disconnect([=](auto &session_ptr) {
            asio2::ignore_unused(session_ptr);
        }).bind_upgrade([](auto &session_ptr) {
            LOGI("File transfer upgrade");
        }).bind_start([&]() {
            LOGI("File transfer start");
        }).bind_stop([&]() {
            LOGI("File transfer stop");
        });

        server_->start("0.0.0.0", settings_->file_transfer_port_, settings_->file_transfer_path_);
    }

    void FileTransferChannel::Exit() {

    }

    void FileTransferChannel::ParseMessage(std::string_view _data) {
        auto msg = std::make_shared<tc::Message>();
        std::string data(_data.data(), _data.size());
        if (!msg->ParseFromString(data)) {
            LOGE("Parse proto message failed");
            return;
        }
        LOGI("Received type: {}", (int) msg->type());
        if (msg->type() == MessageType::kFileTransfer) {
            auto fs = msg->file_transfer();
            if (fs.state() == FileTransfer_FileTransferState_kRequestFileTransfer) {
                // 1. check file state
                // 2. response
                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                resp_msg.mutable_resp_file_transfer()->set_state(RespFileTransfer_FileTransferRespState_kTransferReady);
                auto proto_msg = resp_msg.SerializeAsString();
                // 导致出问题
                PostBinaryMessage(proto_msg);

            } else if (fs.state() == FileTransfer_FileTransferState_kTransferring) {
                auto data_size = fs.data().size();
                auto progress = data_size + fs.transferred_size();
                auto total = fs.filesize();
                LOGI("data size: {}, progress: {}", data_size, (progress * 1.0f / total));
            }
        }
    }

    void FileTransferChannel::PostBinaryMessage(const std::string &msg) {
        thread_->Post([=]() {
            if (session_ && server_ && server_->is_started()) {
                LOGI("send back...");
                session_->async_send(msg);
            }
        });
    }
}
