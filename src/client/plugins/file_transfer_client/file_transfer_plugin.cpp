//
// Created RGAA on 15/11/2024.
//

#include "file_transfer_plugin.h"
#include "tc_message.pb.h"
#include "tc_label.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "tc_qt_widget/no_margin_layout.h"
#include "client/plugins/ct_plugin_ids.h"
#include "client/plugins/ct_plugin_events.h"
#include "client/plugins/ct_app_events.h"
#include "src/core/file_sdk_interface.h"
#include "src/widget/file_trans_widget.h"
#include "src/core/file_trans_interface.h"

void* GetInstance() {
    static tc::FileTransferPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{

    std::string FileTransferPlugin::GetPluginId() {
        return kClientFileTransferPluginId;
    }

    std::string FileTransferPlugin::GetPluginName() {
        return "File Transfer";
    }

    std::string FileTransferPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t FileTransferPlugin::GetVersionCode() {
        return 110;
    }

    void FileTransferPlugin::On1Second() {
        ClientPluginInterface::On1Second();
    }
    
    bool FileTransferPlugin::OnCreate(const tc::ClientPluginParam& param) {
        ClientPluginInterface::OnCreate(param);
        plugin_type_ = ClientPluginType::kUtil;

        if (!IsPluginEnabled()) {
            return true;
        }

        root_widget_->resize(1366, 768);
        root_widget_->show();

        file_trans_interface_ = std::make_shared<FileTransInterface>(this);
        auto layout = new NoMarginHLayout();
        layout->addWidget(file_trans_interface_->GetFileTransWidget());
        root_widget_->setLayout(layout);
        return true;
    }

    void FileTransferPlugin::OnMessage(std::shared_ptr<Message> msg) {
        ClientPluginInterface::OnMessage(msg);
        if (file_trans_interface_) {
            file_trans_interface_->OnProtoMessage(msg);
        }
    }

    void FileTransferPlugin::DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) {
        ClientPluginInterface::DispatchAppEvent(event);
        LOGI("AppEvent: {}", (int)event->evt_type_);
    }

}
