//
// Created by RGAA on 8/08/2024.
//

#ifndef GAMMARAYPC_FS_FILE_H
#define GAMMARAYPC_FS_FILE_H

#include "tc_common_new/file.h"

namespace tc
{

    using SendTask = std::function<bool(const std::string& proto_msg, uint64_t total_size, uint64_t offset_size)>;

    class FsFile {
    public:
        FsFile(const QString& path, int read_block_size);
        bool Send(SendTask && task);
        [[nodiscard]] bool IsOpen() const;

    private:
        std::string MakeTransferMessage(uint64_t offset, std::shared_ptr<Data>&& data) const;

    public:
        std::string file_id_;
        QString file_path_;
        QString file_name_;
        QString ref_path_; // xx/xx/xx.zip
        QString ref_folder_; // xx/xx/
        QString base_folder_name_;
        std::shared_ptr<File> file_ = nullptr;
        int read_block_size_ = 0;
        uint64_t file_size_ = -1;
    };

}

#endif //GAMMARAYPC_FS_FILE_H
