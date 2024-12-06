//
// Created RGAA on 15/11/2024.
//

#include "frame_debugger_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/file.h"
#include "render/plugins/plugin_ids.h"

namespace tc
{

    std::string FrameDebuggerPlugin::GetPluginId() {
        return kFrameDebuggerPluginId;
    }

    std::string FrameDebuggerPlugin::GetPluginName() {
        return "Frame Debugger";
    }

    std::string FrameDebuggerPlugin::GetVersionName() {
        return plugin_version_name_;
    }

    uint32_t FrameDebuggerPlugin::GetVersionCode() {
        return plugin_version_code_;
    }

    std::string FrameDebuggerPlugin::GetPluginDescription() {
        return plugin_desc_;
    }

    void FrameDebuggerPlugin::On1Second() {

    }

    bool FrameDebuggerPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);
        return true;
    }

    bool FrameDebuggerPlugin::OnDestroy() {
        if (encoded_video_file_) {
            encoded_video_file_->Close();
        }
        return true;
    }

    void FrameDebuggerPlugin::OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height) {
        encoded_video_type_ = type;
        std::string encoded_video_file_name = std::format("1_encoded_video.{}", (type == GrPluginEncodedVideoType::kH264) ? "h264" : "h265");
        if (File::Exists(encoded_video_file_name)) {
            File::Delete(encoded_video_file_name);
        }
        encoded_video_file_ = File::OpenForAppendB(encoded_video_file_name);
    }

    void FrameDebuggerPlugin::OnEncodedVideoFrame(const GrPluginEncodedVideoType& video_type,
                                     const std::shared_ptr<Data>& data,
                                     uint64_t frame_index,
                                     int frame_width,
                                     int frame_height,
                                     bool key) {
        encoded_video_file_->Append(data);
    }

}
