//
// Created by RGAA on 8/08/2024.
//

#include "fs_file.h"
#include "tc_common_new/file.h"
#include "tc_message.pb.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/log.h"

namespace tc
{

    FsFile::FsFile(const QString& path, int read_block_size) {
        this->file_ = File::OpenForReadB(path.toStdString());
        this->read_block_size_ = read_block_size;
        this->file_path_ = path;
        if (this->file_->IsOpen()) {
            this->file_size_ = this->file_->Size();
            this->file_name_ = QString::fromStdString(this->file_->FileName());
        }
        this->file_id_ = MD5::Hex(this->file_path_.toStdString() + std::to_string(TimeExt::GetCurrentTimestamp()));
    }

    bool FsFile::Send(SendTask&& task) {
        if (!this->file_->IsOpen()) {
            return false;
        }
        this->file_->ReadAll([=, this](uint64_t offset, DataPtr&& data) -> bool {
            auto msg = MakeTransferMessage(offset, std::move(data));
            return task(msg, this->file_size_, offset);
        });
        return true;
    }

    bool FsFile::IsOpen() const {
        return file_ && file_->IsOpen();
    }

    std::string FsFile::MakeTransferMessage(uint64_t offset, std::shared_ptr<Data>&& data) const {
        tc::Message msg;
        msg.set_type(MessageType::kFileTransfer);
        auto fs = msg.mutable_file_transfer();
        fs->set_id(this->file_id_);
        fs->set_state(FileTransfer_FileTransferState_kTransferring);
        fs->set_file_type(FileTransfer_FileType_kFile);
        fs->set_relative_path("");
        fs->set_filename(this->file_->FileName());
        fs->set_data(data->AsString());
        fs->set_filesize(this->file_size_);
        fs->set_transferred_size(offset);
        fs->set_file_md5("");
        fs->set_ref_folder(this->ref_folder_.toStdString());
        fs->set_ref_path(this->ref_path_.toStdString());
        LOGI("Send, ref folder: {}, ref path: {}", this->ref_folder_.toStdString(), this->ref_path_.toStdString());
        return msg.SerializeAsString();
    }
}