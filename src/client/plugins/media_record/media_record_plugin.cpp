//
// Created RGAA on 15/11/2024.
//

#include "media_record_plugin.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "plugin_interface/ct_plugin_context.h"
#include "client/plugins/ct_plugin_ids.h"
#include "client/plugins/ct_plugin_events.h"
#include "client/plugins/ct_app_events.h"
#include "media_recorder.h"
#include "ct_const_def.h"

#include <qpushbutton.h>

void* GetInstance() {
    static tc::MediaRecordPluginClient plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string MediaRecordPluginClient::GetPluginId() {
        return kMediaRecordPluginId;
    }

    std::string MediaRecordPluginClient::GetPluginName() {
        return "Media Record";
    }

    std::string MediaRecordPluginClient::GetVersionName() {
        return "1.1.0";
    }

    uint32_t MediaRecordPluginClient::GetVersionCode() {
        return 110;
    }

    void MediaRecordPluginClient::On1Second() {
        
        ClientPluginInterface::On1Second();

        auto event = std::make_shared<ClientPluginTestEvent>();
        event->message_ = "///1Second///";
        //CallbackEvent(event);
    }
    
    bool MediaRecordPluginClient::OnCreate(const tc::ClientPluginParam& param) {
        ClientPluginInterface::OnCreate(param);
        plugin_type_ = ClientPluginType::kUtil;

        if (!IsPluginEnabled()) {
            return true;
        }

        for (int index = 0; index < kMaxGameViewCount; ++index) {
            auto recorder = MediaRecorder::Make(this);
            recorder->SetIndex(index);
            media_recorders_.emplace_back(recorder);
        }

        root_widget_->hide();
        root_widget_->setWindowTitle("Media Record");
        auto layout = new QVBoxLayout(root_widget_);
        auto btn = new QPushButton("Start Record", root_widget_);
        btn->setFixedSize(80, 40);
        layout->addWidget(btn);

        return true;
    }

    void MediaRecordPluginClient::OnMessage(std::shared_ptr<Message> msg) {
        ClientPluginInterface::OnMessage(msg);
        if (!recording_) {
            return;
        }

        plugin_context_->PostWorkTask([this, msg]() {
            if (msg->type() == tc::kVideoFrame) {
                const auto& video_frame = msg->video_frame();
                if (video_frame.key()) {
                    LOGI("video frame index: {}, {}x{}, key: {}", video_frame.frame_index(),
                        video_frame.frame_width(), video_frame.frame_height(), video_frame.key());
                }

                int v_idx = video_frame.mon_index();
                if (media_recorders_.size() > v_idx) {
                    media_recorders_[v_idx]->RecvVideoFrame(video_frame);
                }
                else {
                    LOGW("video_frame index : {}, Exceeded the maximum limit", v_idx);
                }
            }
            else if (msg->type() == tc::kAudioFrame) {
                const auto& audio_frame = msg->audio_frame();
                for (auto& media_recorder: media_recorders_) {
                    media_recorder->RecvAudioFrame(audio_frame);
                }
            }
        });
    }

    void MediaRecordPluginClient::DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) {
        ClientPluginInterface::DispatchAppEvent(event);
        LOGI("AppEvent: {}", (int)event->evt_type_);
    }

    void MediaRecordPluginClient::StartRecord() {
        recording_ = true;
    }

    void MediaRecordPluginClient::EndRecord() {
        recording_ = false;
        plugin_context_->PostWorkTask([=, this]() {
            for (auto& media_recorder : media_recorders_) {
                media_recorder->EndRecord();
            } 
        });
    }

    std::string MediaRecordPluginClient::GetScreenRecordingPath() const {
        return screen_recording_path_;
    }
}
