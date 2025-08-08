//
// Created by RGAA  on 8/07/2024.
//

#include "file_transfer.h"
#include "render_panel/gr_settings.h"
#include "render_panel/gr_context.h"
#include "tc_common_new/log.h"
#include "tc_message.pb.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"

namespace tc
{

    FileTransferChannel::FileTransferChannel(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<asio2::http_session>& sess) {
        context_ = ctx;
        sess_ = sess;
        settings_ = GrSettings::Instance();
    }

    void FileTransferChannel::OnConnected() {

    }

    void FileTransferChannel::OnDisConnected() {
        if (transferring_file_) {
            transferring_file_.reset();
            transferring_file_ = nullptr;
        }
    }

    void FileTransferChannel::ParseBinaryMessage(std::string_view _data) {
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
                LOGI("Ref folder: {}", fs.ref_folder());
                std::string file_path;
                // with folder ?
                if (!fs.ref_folder().empty()) {
                    QString folder_path = QString::fromStdString(settings_->file_transfer_folder_ + "/" + fs.ref_folder());
                    QDir dir(folder_path);
                    if (!dir.exists()) {
                        if (dir.mkpath(".")) {
                        } else {
                            LOGE("Create folder failed: {}", folder_path.toStdString());
                            return;
                        }
                    }
                    file_path = settings_->file_transfer_folder_ + "/" + fs.ref_path();
                } else {
                    // 1. check file state
                    file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                }
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
                resp_file_transfer->set_timestamp(TimeUtil::GetCurrentTimestamp());
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);

            } else if (fs.state() == FileTransfer::kTransferring) {
                auto data_size = fs.data().size();
                auto recv_size = data_size + fs.transferred_size();
                auto total = fs.filesize();
                auto progress = (recv_size * 1.0f / total);
                //LOGI("data size: {}, progress: {}", data_size, progress);
                std::string file_path;
                if (!fs.ref_path().empty()) {
                    file_path = settings_->file_transfer_folder_ + "/" + fs.ref_path();
                } else {
                    file_path = settings_->file_transfer_folder_ + "/" + fs.filename();
                }
                if (fs.transferred_size() == 0) {
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
                resp_file_transfer->set_timestamp(TimeUtil::GetCurrentTimestamp());
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
                resp_file_transfer->set_timestamp(TimeUtil::GetCurrentTimestamp());
                resp_file_transfer->set_filesize(fs.filesize());
                auto proto_msg = resp_msg.SerializeAsString();
                PostBinaryMessage(proto_msg);
            }
        }
    }

    void FileTransferChannel::PostBinaryMessage(const std::string &msg) {
        if (sess_) {
            sess_->async_send(msg);
        }
    }
}
