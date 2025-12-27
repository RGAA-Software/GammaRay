//
// Created by RGAA on 29/05/2025.
//

#include "file_transfer_record.h"
#include "tc_common_new/time_util.h"
#include "tc_3rdparty/json/json.hpp"
#include <format>

using namespace nlohmann;

namespace tc
{

    std::string FileTransferRecord::AsString() {
        return std::format("Begin: {}, End: {}, Controller device: {}, Controlled device: {}, direction: {}, file detail: {}",
                           TimeUtil::FormatTimestamp(begin_), TimeUtil::FormatTimestamp(end_), visitor_device_,target_device_, direction_, file_detail_);
    }

    std::string FileTransferRecord::AsJson() {
        nlohmann::json obj;
        obj["the_file_id"] = the_file_id_;
        obj["begin"] = TimeUtil::FormatTimestamp(begin_);
        obj["end"] = TimeUtil::FormatTimestamp(end_);
        obj["visitor_device"] = visitor_device_;
        obj["target_device"] = target_device_;
        obj["direction"] = direction_;
        obj["file_detail"] = file_detail_;
        return obj.dump(2);
    }

    std::string FileTransferRecord::AsJson2() {
        nlohmann::json obj;
        obj["the_file_id"] = the_file_id_;
        obj["begin"] = begin_;
        obj["end"] = end_;
        obj["visitor_device"] = visitor_device_;
        obj["target_device"] = target_device_;
        obj["direction"] = direction_;
        obj["file_detail"] = file_detail_;
        return obj.dump(2);
    }
}