//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_FILE_TRANSFER_RECORD_H
#define GAMMARAY_FILE_TRANSFER_RECORD_H

#include <string>

namespace tc
{

    class FileTransferRecord {
    public:
        bool IsValid() {
            return !controller_device_.empty() && !controlled_device_.empty();
        }
    public:
        std::string conn_type_;
        int64_t begin_{0};
        int64_t end_{0};
        std::string account_;
        std::string controller_device_;
        std::string controlled_device_;
        std::string direction_;
        std::string file_detail_;
    };

}

#endif //GAMMARAY_FILE_TRANSFER_RECORD_H
