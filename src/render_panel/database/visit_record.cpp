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
        return std::format("Conn Type: {}, Begin: {}, End: {}, Duration: {}, Visitor device: {}, Target device: {}",
                           conn_type_,
                           TimeUtil::FormatTimestamp(begin_),
                           TimeUtil::FormatTimestamp(end_),
                           TimeUtil::FormatSecondsToDHMS(duration_),
                           visitor_device_,
                           target_device_);
    }

    std::string VisitRecord::AsJson() {
        nlohmann::json obj;
        obj["conn_id"] = conn_id_;
        obj["stream_id"] = stream_id_;
        obj["conn_type"] = conn_type_;
        obj["begin"] = TimeUtil::FormatTimestamp(begin_);
        obj["end"] = TimeUtil::FormatTimestamp(end_);
        obj["duration"] = TimeUtil::FormatSecondsToDHMS(duration_);
        obj["visitor_device"] = visitor_device_;
        obj["target_device"] = target_device_;
        return obj.dump();
    }

    std::string VisitRecord::AsJson2() {
        nlohmann::json obj;
        obj["conn_id"] = conn_id_;
        obj["stream_id"] = stream_id_;
        obj["conn_type"] = conn_type_;
        obj["begin"] = begin_;
        obj["end"] = end_;
        obj["duration"] = duration_;
        obj["visitor_device"] = visitor_device_;
        obj["target_device"] = target_device_;
        return obj.dump();
    }
}