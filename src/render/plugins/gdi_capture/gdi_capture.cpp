#include "gdi_capture.h"
#include <algorithm>
#include <iostream>
#include <timeapi.h>
#include "tc_common_new/string_ext.h"
#include "tc_common_new/message_notifier.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/monitors.h"
#include "tc_common_new/image.h"
#include "tc_common_new/win32/win_helper.h"
#include "tc_capture_new/capture_message.h"
#include "tc_common_new/math_helper.h"
#include "plugin_interface/gr_plugin_events.h"
#include "gdi_capture_plugin.h"
#include "tc_common_new/win32/d3d_debug_helper.h"


namespace tc
{

    std::shared_ptr<GdiCapture> GdiCapture::Make(GdiCapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info) {
        return std::make_shared<GdiCapture>(plugin, my_monitor_info);
    }

    GdiCapture::GdiCapture(GdiCapturePlugin* plugin, const CaptureMonitorInfo& my_monitor_info)
        : PluginDesktopCapture(my_monitor_info) {
        plugin_ = plugin;
        fps_stat_ = std::make_shared<FpsStat>();
        LOGI("GdiCapture my monitor info: {}", my_monitor_info.Dump());
    }

    GdiCapture::~GdiCapture() {
       
    }

    bool GdiCapture::Init() {
        init_success_ = false;     
        screen_dc_ = GetDC(NULL); // 整个虚拟屏幕的设备上下文, GetDC 是采集整个虚拟屏幕的画面,GDI 作为托底采集,就采集整个虚拟桌面就可以
        //screen_dc_ = CreateDC(NULL, R"(\\.\DISPLAY1)", NULL, NULL); // CreateDC 可以采集特定屏幕的画面
        if (!screen_dc_) {
            LOGW("GdiCapture GetDC failed.");
            return false;
        }

        memory_dc_ = CreateCompatibleDC(screen_dc_);
        if (!memory_dc_) {
            LOGW("GdiCapture CreateCompatibleDC failed.");
            return false;
        }

        if (SetStretchBltMode(memory_dc_, COLORONCOLOR) == 0) { // 使用 COLORONCOLOR 可以提高图像缩放的速度，适合在不需要透明效果的情况下使用。
            LOGW("SetStretchBltMode failed.");
        }

        // 创建兼容位图
        bit_map_ = CreateCompatibleBitmap(screen_dc_, my_monitor_info_.virtual_desktop_width_, my_monitor_info_.virtual_desktop_height_);
        if (!bit_map_) {
            LOGW("CreateCompatibleBitmap failed.");
            return false;
        }

        // 选择位图到内存 DC 中
        SelectObject(memory_dc_, bit_map_);
        init_success_ = true;
        LOGI("GdiCapture Init OK.");
        return true;
    }

    bool GdiCapture::IsInitSuccess() {
        return init_success_;
    }

    bool GdiCapture::Exit() {
        init_success_ = false;

        DeleteObject(bit_map_);
        bit_map_ = nullptr;

        DeleteDC(screen_dc_);
        screen_dc_ = nullptr;

        ReleaseDC(NULL, memory_dc_);
        memory_dc_ = nullptr;

        return true;
    }

    bool GdiCapture::CaptureNextFrame() {
        // 复制整个虚拟屏幕的内容到内存 DC
        BitBlt(memory_dc_, 0, 0, my_monitor_info_.virtual_desktop_width_, my_monitor_info_.virtual_desktop_height_, screen_dc_, my_monitor_info_.virtual_desktop_left_, 
            my_monitor_info_.virtual_desktop_top_, SRCCOPY);
        BITMAP bmp;
        GetObject(bit_map_, sizeof(BITMAP), &bmp);
        
        BITMAPINFOHEADER bi;
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = bmp.bmWidth;
        bi.biHeight = -bmp.bmHeight; // 负值表示位图是自上而下的
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        if (bmp.bmWidthBytes != bmp.bmWidth * 4) {
            LOGW("bmp.bmWidthBytes != bmp.bmWidth * 4. bmp.bmWidthBytes: {}, bmp.bmWidth: {}", bmp.bmWidthBytes, bmp.bmWidth);
        }

        DWORD dwBmpSize = bmp.bmWidthBytes * bmp.bmHeight;

        DataPtr data_ptr = Data::Make(nullptr, dwBmpSize);
        if (!data_ptr) {
            LOGE("DataPtr Make error!");
            return false;
        }

        int ret = GetDIBits(memory_dc_, bit_map_, 0, (UINT)bmp.bmHeight, data_ptr->DataAddr(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        if (ret == 0) {
            LOGW("GetDIBits failed.");
            return false;
        }

        CaptureVideoFrame cap_video_frame{};
        cap_video_frame.type_ = kCaptureVideoFrame;
        cap_video_frame.capture_type_ = kCaptureVideoByBitmapData;
        cap_video_frame.data_length = 0;
        cap_video_frame.frame_width_ = bmp.bmWidth;
        cap_video_frame.frame_height_ = bmp.bmHeight;
        cap_video_frame.frame_index_ = GetFrameIndex();
        cap_video_frame.raw_image_ = Image::Make(data_ptr, bmp.bmWidth, bmp.bmHeight, RawImageType::kBGRA);
        memcpy(cap_video_frame.display_name_, kVirtualDesktopNameSign.c_str(), kVirtualDesktopNameSign.size());
        
        auto event = std::make_shared<GrPluginCapturedVideoFrameEvent>();
        event->frame_ = cap_video_frame;
        this->plugin_->CallbackEvent(event);

        // fps tick
        fps_stat_->Tick();

        // capture gaps
        auto curr_timestamp = (int64_t)TimeUtil::GetCurrentTimestamp();
        if (last_captured_timestamp_ == 0) {
            last_captured_timestamp_ = curr_timestamp;
        }
        auto diff = curr_timestamp - last_captured_timestamp_;
        if (capture_gaps_.size() >= 180) {
            capture_gaps_.pop_front();
        }
        capture_gaps_.push_back((int32_t)diff);
        last_captured_timestamp_ = curr_timestamp;

#if 0   // save rgb to file
        static int frame_index_ = -1;
        ++frame_index_;
        std::wstring file_name = std::wstring(L"desktop_") + std::to_wstring(frame_index_ % 12) + L".bgra";
        HANDLE hFile = CreateFileW(file_name.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        DWORD dwBytesWritten;
        WriteFile(hFile, data_ptr->DataAddr(), dwBmpSize, &dwBytesWritten, NULL);
#endif

        return true;
    }

    void GdiCapture::Start() {
        capture_thread_ = std::thread([this] {
            while (!stop_flag_) {
                if (!this->Init()) {
                    LOGE("gdi capture init failed for target: {}, will try again.", my_monitor_info_.name_);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                else {
                    break;
                }
            }

            if (gdi_init_success_callback_) {
                gdi_init_success_callback_();
            }

            Capture();
        });
    }

    void GdiCapture::Capture() {
        while (!stop_flag_) {
            if (pausing_ || plugin_->DontHaveConnectedClientsNow()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(17));
                continue;
            }

            if (!WinHelper::InputDesktopSelected()) {
                if (!WinHelper::SelectInputDesktop()) {
                    LOGE("GDI capture SelectInputDesktop error.");
                }
                // 切换桌面后，需要重新初始化 GDI相关
                this->Exit();
                this->Init();
                plugin_->InsertIdr();
            }
            CaptureNextFrame();
        }
    }

    int GdiCapture::GetFrameIndex() {
        monitor_frame_index_++;
        return monitor_frame_index_;
    }

    bool GdiCapture::StartCapture() {
        this->Start();
        return true;
    }

    bool GdiCapture::PauseCapture() {
        pausing_ = true;
        return true;
    }

    void GdiCapture::ResumeCapture() {
        pausing_ = false;
        plugin_->InsertIdr();
    }

    void GdiCapture::StopCapture() {
        stop_flag_ = true;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
        this->Exit();
    }

    void GdiCapture::RefreshScreen() {
        PluginDesktopCapture::RefreshScreen();
        used_cache_times_ = 0;
    }

    bool GdiCapture::IsPrimaryMonitor() {
        return is_primary_monitor_;
    }

    int GdiCapture::GetCapturingFps() {
        return fps_stat_->value();
    }

} // tc