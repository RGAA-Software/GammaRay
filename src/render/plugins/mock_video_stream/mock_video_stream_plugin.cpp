//
// Created RGAA on 15/11/2024.
//

#include "mock_video_stream_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render/plugins/plugin_ids.h"
#include "plugin_interface/gr_plugin_context.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/image.h"
#include "tc_common_new/data.h"
#include "tc_common_new/log.h"

namespace tc
{
    std::string MockVideoStreamPlugin::GetPluginId() {
        return kMockVideoStreamPluginId;
    }

    std::string MockVideoStreamPlugin::GetPluginName() {
        return "Mock Video Frame Plugin";
    }

    std::string MockVideoStreamPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t MockVideoStreamPlugin::GetVersionCode() {
        return 110;
    }

    bool MockVideoStreamPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrDataProviderPlugin::OnCreate(param);
        return true;
    }

    void MockVideoStreamPlugin::On1Second() {
        PostWorkTask([=, this]() {
            ReGenerate();
        });
    }

    void MockVideoStreamPlugin::StartProviding() {
        mock_image_ = cv::Mat(height_, width_, CV_8UC4);
        ReGenerate();
        plugin_context_->StartTimer(33, [=, this]() {
            frame_index_++;
            auto event = std::make_shared<GrPluginRawVideoFrameEvent>();
            auto size = mock_image_.cols * mock_image_.rows * mock_image_.channels();
            event->image_ = Image::Make(Data::Make((char*)mock_image_.data, size), width_, height_);
            event->frame_index_ = frame_index_;
            event->frame_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
            CallbackEvent(event);
        });
    }

    void MockVideoStreamPlugin::StopProviding() {

    }

    void MockVideoStreamPlugin::ReGenerate() {
        if (mock_image_.empty()) {
            return;
        }
        for (int y = 0; y < height_; y++) {
            for (int x = 0; x < width_; x++) {
                uchar r = rand() % 256;
                uchar g = rand() % 256;
                uchar b = rand() % 256;
                uchar a = rand() % 256;
                mock_image_.at<cv::Vec4b>(y, x) = cv::Vec4b(b, g, r, a);
            }
        }
        //cv::imwrite("1234.png", mock_image_);
    }

}
