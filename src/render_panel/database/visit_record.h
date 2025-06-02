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
        [[nodiscard]] bool IsValid() const {
            return !controller_device_.empty() && !controlled_device_.empty();
        }

        std::string AsString();
        std::string AsJson();

    public:
        int id_{};
        std::string conn_type_;
        // unit: ms
        int64_t begin_{0};
        // unit: ms
        int64_t end_{0};
        // unit: S
        int64_t duration_{0};
        std::string account_;
        std::string controller_device_;
        std::string controlled_device_;
    };

}

#endif //GAMMARAY_VISIT_RECORD_H
