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
            if (!asio2::get_last_error()) {
                session_ = session_ptr;
                session_ptr->ws_stream().binary(true);
                session_ptr->ws_stream().set_option(
                    websocket::stream_base::decorator([session_ptr](websocket::response_type &rep) {
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
        }).bind_recv([=](auto &session_ptr, std::string_view data) {
            this->ParseMessage(data);
        }).bind_connect([](auto &session_ptr) {

        }).bind_disconnect([=](auto &session_ptr) {
            if (transferring_file_) {
                transferring_file_.reset();
                transferring_file_ = nullptr;
            }
        }).bind_upgrade([](auto &session_ptr) {

        }).bind_start([&]() {

        }).bind_stop([&]() {

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

        if (msg->type() == MessageType::kFileTransfer) {
            auto fs = msg->file_transfer();

            auto func_transfer_failed = [=, this]() {
                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                auto resp_file_transfer = resp_msg.mutable_resp_file_transfer();
                resp_file_transfer->set_id(fs.id());
                resp_file_transfer->set_state(RespFileTransfer::kTransferFailed);
                resp_file_transfer->set_filename(fs.filename());
                PostBinaryMessage(resp_msg.SerializeAsString());
            };

            if (fs.state() == FileTransfer::kRequestFileTransfer) {
                // 1. check file state
                auto file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                bool ready_to_transfer;
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
                auto resp_file_transfer = resp_msg.mutable_resp_file_transfer();
                resp_file_transfer->set_id(fs.id());
                if (ready_to_transfer) {
                    resp_file_transfer->set_state(RespFileTransfer::kTransferReady);
                } else {
                    resp_file_transfer->set_state(RespFileTransfer::kFileDeleteFailed);
                }
                resp_file_transfer->set_filename(fs.filename());
                resp_file_transfer->set_local_filepath(fs.local_filepath());
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);

            } else if (fs.state() == FileTransfer::kTransferring) {
                auto data_size = fs.data().size();
                auto recv_size = data_size + fs.transferred_size();
                auto total = fs.filesize();
                auto progress = (recv_size * 1.0f / total);
                //LOGI("data size: {}, progress: {}", data_size, progress);
                if (fs.transferred_size() == 0) {
                    auto file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                    transferring_file_ = File::OpenForAppendB(file_path);
                }
                if (!transferring_file_ || !transferring_file_->IsOpen()) {
                    LOGE("File open failed: {}", settings_->file_transfer_folder_+"/"+fs.filename());
                    func_transfer_failed();
                    return;
                }
                transferring_file_->Append(fs.data());

                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                auto resp_file_transfer = resp_msg.mutable_resp_file_transfer();
                resp_file_transfer->set_id(fs.id());
                resp_file_transfer->set_state(RespFileTransfer::kTransferring);
                resp_file_transfer->set_filename(fs.filename());
                resp_file_transfer->set_local_filepath(fs.local_filepath());
                resp_file_transfer->set_filesize(fs.filesize());
                resp_file_transfer->set_transferred_size(fs.transferred_size());
                resp_file_transfer->set_progress(progress);
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);

            }  else if (fs.state() == FileTransfer::kTransferOver) {
                LOGI("File transfer over: {}", fs.filename());
                if (transferring_file_ && transferring_file_->IsOpen()) {
                    transferring_file_.reset();
                    transferring_file_ = nullptr;
                }

                tc::Message resp_msg;
                resp_msg.set_type(MessageType::kRespFileTransfer);
                auto resp_file_transfer = resp_msg.mutable_resp_file_transfer();
                resp_file_transfer->set_id(fs.id());
                resp_file_transfer->set_state(RespFileTransfer::kTransferSuccess);
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);
            }
        }
    }

    void FileTransferChannel::PostBinaryMessage(const std::string &msg) {
        if (session_ && server_ && server_->is_started()) {
            session_->async_send(msg);
        }
    }
}
