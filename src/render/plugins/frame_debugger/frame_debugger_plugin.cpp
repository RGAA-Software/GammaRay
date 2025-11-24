//
// Created RGAA on 15/11/2024.
//

#include "frame_debugger_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "plugin_interface/gr_plugin_context.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
#include "render/plugins/plugin_ids.h"
#include "tc_common_new/folder_util.h"
#include "tc_common_new/string_util.h"

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
        if (save_encoded_video_) {
            lbl_frame_ = new QLabel(root_widget_);
            lbl_frame_->setFixedSize(960, 540);
            auto layout = new QVBoxLayout();
            layout->addWidget(lbl_frame_);
            root_widget_->setLayout(layout);
            root_widget_->show();
        }
        return true;
    }

    bool FrameDebuggerPlugin::OnDestroy() {
        for (const auto& [k, v] : encoded_video_files_) {
            v->Close();
        }
        return GrStreamPlugin::OnDestroy();
    }

    static QPixmap RgbaToPixmap(const uint8_t* data, int width, int height) {
        QImage img(data, width, height, QImage::Format_RGBA8888);
        return QPixmap::fromImage(img.copy());
    }

    void FrameDebuggerPlugin::OnRawVideoFrameRgba(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) {
        if (!image->data || !lbl_frame_ || !IsPluginEnabled()) {
            return;
        }
        plugin_context_->PostUITask([=, this]() {
            auto pixmap = RgbaToPixmap((uint8_t*)image->data->CStr(), image->width, image->height);
            lbl_frame_->setPixmap(pixmap);
        });
    }

    void FrameDebuggerPlugin::OnVideoEncoderCreated(const std::string& mon_name, const GrPluginEncodedVideoType& type, int width, int height) {
        if (mon_name.size() <= 4) {
            return;
        }
        encoded_video_type_ = type;
        if (new_client_in_) {
            for (const auto& [mn, file] : encoded_video_files_) {
                file->Close();
            }
            new_client_in_ = false;
        }
        auto part_name = TimeUtil::FormatTimestamp2(TimeUtil::GetCurrentTimestamp());
        auto folder_path = StringUtil::ToUTF8(FolderUtil::GetProgramDataPath());
        auto display_name = mon_name.substr(4);
        std::string encoded_video_file_name = std::format("{}/gr_data/enc_{}_{}.{}", folder_path, display_name, part_name, (type == GrPluginEncodedVideoType::kH264) ? "h264" : "h265");
        if (File::Exists(encoded_video_file_name)) {
            File::Delete(encoded_video_file_name);
        }
        if (save_encoded_video_) {
            auto file = File::OpenForAppendB(encoded_video_file_name);
            encoded_video_files_[mon_name] = file;
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
            if (encoded_video_files_.contains(mon_name)) {
                encoded_video_files_[mon_name]->Append(data);
            }
        }
    }

    void FrameDebuggerPlugin::OnNewClientConnected(const std::string& visitor_device_id, const std::string& stream_id, const std::string& conn_type) {
        new_client_in_ = true;
    }

}
