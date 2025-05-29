//
// Created RGAA on 15/11/2024.
//

#include "media_record_plugin.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "client/plugins/ct_plugin_ids.h"
#include "client/plugins/ct_plugin_events.h"
#include "client/plugins/ct_app_events.h"
#include "media_recorder.h"

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
        //root_widget_->hide();            
        media_recorder_ = MediaRecorder::Make(this);

        root_widget_->show();
        root_widget_->setWindowTitle("Media Record");
        auto layout = new QVBoxLayout(root_widget_);
        auto btn = new QPushButton("Start Record", root_widget_);
        btn->setFixedSize(80, 40);
        layout->addWidget(btn);
        root_widget_->hide();

        return true;
    }

    void MediaRecordPluginClient::OnMessage(std::shared_ptr<Message> msg) {
        ClientPluginInterface::OnMessage(msg);
        //LOGI("MediaRecordPluginClient OnMessage: {}", (int)msg->type());

        // to do 使用线程 来负责录制


        if (msg->type() == tc::kVideoFrame) {
            const auto& video_frame = msg->video_frame();
            if (video_frame.key()) {
                LOGI("video frame index: {}, {}x{}, key: {}", video_frame.frame_index(),
                     video_frame.frame_width(), video_frame.frame_height(), video_frame.key());
            }
            media_recorder_->RecvVideoFrame(video_frame);

        }
        else if (msg->type() == tc::kAudioFrame) {
            const auto& audio_frame = msg->audio_frame();
            
        }



    }

    void MediaRecordPluginClient::DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) {
        ClientPluginInterface::DispatchAppEvent(event);
        LOGI("AppEvent: {}", (int)event->evt_type_);
    }

    void MediaRecordPluginClient::StartRecord() {
        if (!media_recorder_) {
            return;
        }

        media_recorder_->StartRecord();
    }

    void MediaRecordPluginClient::EndRecord() {
        if (!media_recorder_) {
            return;
        }

        media_recorder_->EndRecord();
    }

    /*void MediaRecordPluginClient::RecvVideoFrame(const VideoFrame& frame) {
        if (!media_recorder_) {
            return;
        }

        media_recorder_->RecvVideoFrame(frame);
    }*/
}
