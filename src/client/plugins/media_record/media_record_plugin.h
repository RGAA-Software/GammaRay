//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_MEDIA_RECORDER_PLUGIN_H
#define GAMMARAY_MEDIA_RECORDER_PLUGIN_H

#include "plugin_interface/ct_media_record_plugin_interface.h"
#include <map>
#include <memory>
//#include "tc_message.pb.h"

namespace tc
{
    class MediaRecorder;

    class MediaRecordPluginClient : public MediaRecordPluginClientInterface {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        bool OnCreate(const tc::ClientPluginParam& param) override;
        void On1Second() override;
        void OnMessage(std::shared_ptr<Message> msg) override;
        void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) override;

        void StartRecord() override;
        void EndRecord() override;
        //void RecvVideoFrame(const VideoFrame& frame) override;

    private:
        std::map<std::string, QLabel*> previewers_;
        std::shared_ptr<MediaRecorder> media_recorder_;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();




#endif //GAMMARAY_UDP_PLUGIN_H
