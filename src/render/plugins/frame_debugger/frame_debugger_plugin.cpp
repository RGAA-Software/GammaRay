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
            root_widget_->setWindowTitle(GetPluginName().c_str());
            content_layout_ = new QHBoxLayout();
            root_widget_->setLayout(content_layout_);
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
        auto pixmap = QPixmap::fromImage(img.copy());
        return pixmap.scaled(960, 540);
    }

    void FrameDebuggerPlugin::OnRawVideoFrameRgba(const std::string& mon_name, uint64_t frame_idx, int frame_width, int frame_height, const std::shared_ptr<Image>& image) {
        if (!image->data || !IsPluginEnabled() || mon_name.empty()) {
            return;
        }
        plugin_context_->PostUITask([=, this]() {
            if (!frames_info_.contains(mon_name)) {
                auto layout = new QVBoxLayout();
                auto lbl_info = new QLabel(root_widget_);
                layout->addWidget(lbl_info);

                auto lbl_frame = new QLabel(root_widget_);
                lbl_frame->setFixedSize(960, 540);
                layout->addWidget(lbl_frame);
                content_layout_->addLayout(layout);

                auto frame_info = std::make_shared<SingleFrameInfo>();
                frame_info->mon_name_ = mon_name;
                frame_info->lbl_info_ = lbl_info;
                frame_info->lbl_frame_ = lbl_frame;
                frames_info_.insert({mon_name, frame_info});
            }

            auto frame_info = frames_info_[mon_name];

            QString msg;
            msg += "idx: " + QString::number(frame_idx) + ", " + QString::number(frame_width) + "x" + QString::number(frame_height);
            frame_info->lbl_info_->setText(msg);
            auto pixmap = RgbaToPixmap((uint8_t*)image->data->CStr(), image->width, image->height);
            frame_info->lbl_frame_->setPixmap(pixmap);
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
