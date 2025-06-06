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
        [[nodiscard]] bool IsValid() const {
            return !visitor_device_.empty();
        }

        std::string AsString();
        std::string AsJson();

    public:
        int id_{0};
        std::string the_file_id_;
        int64_t begin_{0};
        int64_t end_{0};
        std::string visitor_device_;
        std::string target_device_;
        std::string direction_;
        std::string file_detail_;
        bool success_ = false;
    };

}

#endif //GAMMARAY_FILE_TRANSFER_RECORD_H
