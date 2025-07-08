//
// Created by RGAA on 29/05/2025.
//

#ifndef GAMMARAY_VISIT_RECORD_H
#define GAMMARAY_VISIT_RECORD_H

#include <string>

namespace tc
{

    class VisitRecord {
    public:
        [[nodiscard]] bool IsHeaderItem() const {
            return id_ == 0 && visitor_device_.empty() && target_device_.empty();
        }

        std::string AsString();
        std::string AsJson();

    public:
        int id_{};
        std::string conn_id_;
        std::string stream_id_;
        std::string conn_type_;
        // unit: ms
        int64_t begin_{0};
        // unit: ms
        int64_t end_{0};
        // unit: S
        int64_t duration_{0};
        std::string visitor_device_;
        std::string target_device_;
    };

}

#endif //GAMMARAY_VISIT_RECORD_H
