//
// Created RGAA on 15/11/2024.
//

#include "frame_debugger_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
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
        return "Frame debuggers";
    }

    void FrameDebuggerPlugin::On1Second() {

    }

    bool FrameDebuggerPlugin::OnCreate(const tc::GrPluginParam& param) {
        GrPluginInterface::OnCreate(param);
        auto key_save_encoded_video = "save_encoded_video";
        if (HasParam(key_save_encoded_video)) {
            save_encoded_video_ = GetConfigParam<bool>(key_save_encoded_video);
        }
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
        if (new_client_in_) {
            if (encoded_video_file_) {
                encoded_video_file_.reset();
            }
            new_client_in_ = false;
        }
        auto part_name = TimeUtil::FormatTimestamp2(TimeUtil::GetCurrentTimestamp());
        std::string encoded_video_file_name = std::format("gr_data/{}_enc_video.{}", part_name, (type == GrPluginEncodedVideoType::kH264) ? "h264" : "h265");
        if (File::Exists(encoded_video_file_name)) {
            File::Delete(encoded_video_file_name);
        }
        if (save_encoded_video_) {
            encoded_video_file_ = File::OpenForAppendB(encoded_video_file_name);
        }
    }

    void FrameDebuggerPlugin::OnEncodedVideoFrame(const std::string& mon_name,
                                                  const GrPluginEncodedVideoType& video_type,
                                                  const std::shared_ptr<Data>& data,
                                                  uint64_t frame_index,
                                                  int frame_width,
                                                  int frame_height,
                                                  bool key) {
        if (save_encoded_video_) {
            encoded_video_file_->Append(data);
        }
    }

    void FrameDebuggerPlugin::OnNewClientConnected(const std::string& visitor_device_id, const std::string& stream_id, const std::string& conn_type) {
        new_client_in_ = true;
    }

}
