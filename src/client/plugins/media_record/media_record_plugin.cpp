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

namespace tc
{

    std::string MediaRecordPlugin::GetPluginId() {
        return kMediaRecordPluginId;
    }

    std::string MediaRecordPlugin::GetPluginName() {
        return "Media Record";
    }

    std::string MediaRecordPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t MediaRecordPlugin::GetVersionCode() {
        return 110;
    }

    void MediaRecordPlugin::On1Second() {
        ClientPluginInterface::On1Second();

        auto event = std::make_shared<ClientPluginTestEvent>();
        event->message_ = "///1Second///";
        CallbackEvent(event);
    }
    
    bool MediaRecordPlugin::OnCreate(const tc::ClientPluginParam& param) {
        ClientPluginInterface::OnCreate(param);
        plugin_type_ = ClientPluginType::kUtil;

        if (!IsPluginEnabled()) {
            return true;
        }
        //root_widget_->hide();
        root_widget_->show();
        return true;
    }

    void MediaRecordPlugin::OnMessage(std::shared_ptr<Message> msg) {
        ClientPluginInterface::OnMessage(msg);
        //LOGI("OnMessage: {}", (int)msg->type());
    }

    void MediaRecordPlugin::DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) {
        ClientPluginInterface::DispatchAppEvent(event);
        LOGI("AppEvent: {}", (int)event->evt_type_);
    }

}
