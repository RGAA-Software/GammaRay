#pragma once



#include "ct_plugin_interface.h"
//#include "tc_message.pb.h"

namespace tc
{
    

class MediaRecordPluginClientInterface : public ClientPluginInterface {
public:
    //std::string GetPluginId() override;
    //std::string GetPluginName() override;
    //std::string GetVersionName() override;
    //uint32_t GetVersionCode() override;

    //bool OnCreate(const tc::ClientPluginParam& param) override;
    //void On1Second() override;
    //void OnMessage(std::shared_ptr<Message> msg) override;
    //void DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent>& event) override;

    virtual void StartRecord() {};
    virtual void EndRecord() {};
    //virtual void RecvVideoFrame(const VideoFrame& frame) {};

//private:
//    std::map<std::string, QLabel*> previewers_;
//
//    std::shared_ptr<MediaRecorder> media_recorder_;

};

}