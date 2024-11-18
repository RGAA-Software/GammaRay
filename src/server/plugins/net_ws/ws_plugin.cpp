//
// Created RGAA on 15/11/2024.
//

#include "ws_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "ws_server.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"

#include <QFile>

namespace tc
{

    std::string WsPlugin::GetPluginName() {
        return "WS Plugin";
    }

    std::string WsPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t WsPlugin::GetVersionCode() {
        return 110;
    }

    bool WsPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        Logger::InitLog(plugin_file_name_+".log", true);
        LOGI("{} OnCreate", GetPluginName());
        plugin_type_ = GrPluginType::kStream;
        ws_server_ = std::make_shared<WsPluginServer>();
        ws_server_->Start();
        return true;
    }

    bool WsPlugin::OnDestroy() {
        if (ws_server_) {
            ws_server_->Exit();
        }
        return true;
    }

    void WsPlugin::On1Second() {
        GrPluginInterface::On1Second();
        auto evt = std::make_shared<GrPluginKeyboardEvent>();
        evt->plugin_name_ = GetPluginName();
        CallbackEvent(evt);
    }

    bool WsPlugin::IsWorking() {
        return ws_server_ && ws_server_->GetConnectionPeerCount() > 0;
    }

    void WsPlugin::OnVideoEncoderCreated(const tc::GrPluginEncodedVideoType& type, int width, int height) {
        LOGI("OnVideoEncoderCreated: {}, {}x{}", (int)type, width, height);
    }

    void WsPlugin::OnEncodedVideoFrame(const tc::GrPluginEncodedVideoType& video_type,
                             const std::shared_ptr<Data>& data,
                             uint64_t frame_index,
                             int frame_width,
                             int frame_height,
                             bool key) {
    }

    void WsPlugin::OnEncodedVideoFrameInProtobufFormat(const std::string& msg) {
        if (ws_server_) {
            ws_server_->PostVideoMessage(msg);
        }
    }

}
