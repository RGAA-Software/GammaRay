//
// Created by RGAA on 29/05/2025.
//

#include "visit_record.h"
#include <format>
#include "tc_common_new/time_util.h"
#include "tc_3rdparty/json/json.hpp"

namespace tc
{

    std::string VisitRecord::AsString() {
        return std::format("Conn Type: {}, Begin: {}, End: {}, Duration: {}, Account: {}, Controller device: {}, Controlled device: {}",
                           conn_type_,
                           TimeUtil::FormatTimestamp(begin_),
                           TimeUtil::FormatTimestamp(end_),
                           TimeUtil::FormatSecondsToDHMS(duration_),
                           account_,
                           controller_device_,
                           controlled_device_);
    }

    std::string VisitRecord::AsJson() {
        nlohmann::json obj;
        obj["conn_type"] = conn_type_;
        obj["begin"] = TimeUtil::FormatTimestamp(begin_);
        obj["end"] = TimeUtil::FormatTimestamp(end_);
        obj["duration"] = TimeUtil::FormatSecondsToDHMS(duration_);
        obj["account"] = account_;
        obj["controller_device"] = controller_device_;
        obj["controlled_device"] = controlled_device_;
        return obj.dump(2);
    }

}