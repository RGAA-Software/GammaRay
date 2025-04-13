//
// Created by RGAA  on 2024/1/3.
//

#ifndef WIN_CAPTURE_MONITOR_UTIL_H
#define WIN_CAPTURE_MONITOR_UTIL_H

#include <dxgi.h>
#include <string>
#include <string>
#include <sstream>
#include <vector>

namespace tc
{
    using MonitorIndex = uint32_t;

    class SupportedResolution {
    public:
        unsigned long width_ = 0;
        unsigned long height_ = 0;
    };

    class CaptureMonitorInfo {
    public:
        MonitorIndex index_{};
        std::string name_;
        bool attached_desktop_{};
        long top_{};
        long left_{};
        long right_{};
        long bottom_{};

        //to do ´ýÉ¾³ý
        long virtual_top_{};
        long virtual_left_{};
        long virtual_right_{};
        long virtual_bottom_{};
        long virtual_width_;
        long virtual_height_;
        std::vector<SupportedResolution> supported_res_;
    public:

        [[nodiscard]] long Width() const {
            return right_ - left_;
        }

        [[nodiscard]] long Height() const {
            return bottom_ - top_;
        }

        [[nodiscard]] bool Valid() const {
            return !name_.empty() && right_ > left_ && bottom_ > top_;
        }

        [[nodiscard]] std::string Dump() const {
            std::stringstream ss;
            ss << "Monitor index: " << index_ << ", name: " << name_ << std::endl;
            ss << "attached desktop: " << attached_desktop_ << std::endl;
            ss << "left: " << left_ << ", top: " << top_ << ", right: " << right_ << ", bottom: " << bottom_ << std::endl;
            return ss.str();
        }
    };

}
#endif //WIN_CAPTURE_MONITOR_UTIL_H
