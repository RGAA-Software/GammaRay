//
// Created by hy on 8/07/2024.
//

#include "file_transfer.h"
#include "gr_settings.h"
#include "gr_context.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/file.h"

namespace tc
{

    std::shared_ptr<FileTransferChannel> FileTransferChannel::Make(const std::shared_ptr<GrContext> &ctx) {
        return std::make_shared<FileTransferChannel>(ctx);
    }

    FileTransferChannel::FileTransferChannel(const std::shared_ptr<GrContext> &ctx) {
        context_ = ctx;
        settings_ = GrSettings::Instance();
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

        server_->start("0.0.0.0", settings_->file_transfer_port_, settings_->file_transfer_listen_path_);
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
                auto file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                bool ready_to_transfer = false;
                if (!QFile::exists(file_path.c_str())) {
                    ready_to_transfer = true;
                } else {
                    if (File::Delete(file_path)) {
                        ready_to_transfer = true;
                    } else {
                        ready_to_transfer = false;
                        LOGE("Delete file failed: {}", file_path);
                    }
                }

                // 2. response
                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                if (ready_to_transfer) {
                    resp_msg.mutable_resp_file_transfer()->set_state(
                            RespFileTransfer_FileTransferRespState_kTransferReady);
                } else {
                    resp_msg.mutable_resp_file_transfer()->set_state(
                            RespFileTransfer_FileTransferRespState_kFileDeleteFailed);
                }
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);

            } else if (fs.state() == FileTransfer_FileTransferState_kTransferring) {
                auto data_size = fs.data().size();
                auto progress = data_size + fs.transferred_size();
                auto total = fs.filesize();
                LOGI("data size: {}, progress: {}", data_size, (progress * 1.0f / total));
                if (fs.transferred_size() == 0) {
                    auto file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                    transferring_file_ = File::OpenForAppendB(file_path);
                }
                if (!transferring_file_ || !transferring_file_->IsOpen()) {
                    LOGE("File open failed: {}", settings_->file_transfer_folder_+"/"+fs.filename());
                    return;
                }
                transferring_file_->Append(fs.data());

            }  else if (fs.state() == FileTransfer_FileTransferState_kTransferOver) {
                LOGI("File transfer over: {}", fs.filename());
                if (transferring_file_ && transferring_file_->IsOpen()) {
                    transferring_file_.reset();
                    transferring_file_ = nullptr;
                }

                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                resp_msg.mutable_resp_file_transfer()->set_state(RespFileTransfer_FileTransferRespState_kTransferSuccess);
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);
            }
        }
    }

    void FileTransferChannel::PostBinaryMessage(const std::string &msg) {
        if (session_ && server_ && server_->is_started()) {
            LOGI("send back...");
            session_->async_send(msg);
        }
    }
}
