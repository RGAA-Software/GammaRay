//
// Created by RGAA on 10/07/2024.
//

#ifndef GAMMARAYPC_FS_OBJECT_H
#define GAMMARAYPC_FS_OBJECT_H

#include "tc_common_new/file.h"
#include "fs_file.h"
#include <vector>

namespace tc
{
    class File;

    class FileSystemObject {
    public:
        explicit FileSystemObject(const QString& file_path, int read_block_size);
        bool IsFolder() const;

    public:
        // file
        std::shared_ptr<FsFile> fs_file_ = nullptr;

        // folder
        bool is_folder_ = false;
        std::vector<std::shared_ptr<FsFile>> fs_files_;
        QString folder_path_;
    };

}
#endif //GAMMARAYPC_FS_OBJECT_H
