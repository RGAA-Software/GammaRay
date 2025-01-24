//
// Created by RGAA on 10/07/2024.
//

#include "fs_object.h"
#include "tc_common_new/file.h"
#include "tc_message.pb.h"
#include "tc_common_new/md5.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/log.h"

namespace tc
{

    FileSystemObject::FileSystemObject(const QString& file_path, int read_block_size) {
        if (File::IsFolder(file_path.toStdString())) {
            is_folder_ = true;
            folder_path_ = file_path;
            auto dir_name = QDir(folder_path_).dirName();
            LOGI("** Folder Path: {}", folder_path_.toStdString());
            FolderUtil::VisitAllByQt(file_path.toStdString(), [=, this](VisitResult&& result) {
                auto abs_file_path = QString::fromStdWString(result.path_);
                auto filename = QString::fromStdWString(result.name_);
                auto file = std::make_shared<FsFile>(abs_file_path, read_block_size);
                auto start_idx = folder_path_.size()+1;
                file->ref_path_ = dir_name + "/" + abs_file_path.mid(start_idx);
                file->ref_folder_ = dir_name + "/" + abs_file_path.mid(start_idx, abs_file_path.size()-filename.size()-start_idx).trimmed();
                file->base_folder_name_ = dir_name;
                fs_files_.push_back(file);
                LOGI("Name: {}, Path: {}", QString::fromStdWString(result.name_).toStdString(), abs_file_path.toStdString());
                LOGI("Ref path: {}, Ref folder: {}", file->ref_path_.toStdString(), file->ref_folder_.toStdString());
            });

        } else {
            fs_file_ = std::make_shared<FsFile>(file_path, read_block_size);
        }
    }

    bool FileSystemObject::IsFolder() const {
        return is_folder_;
    }

}