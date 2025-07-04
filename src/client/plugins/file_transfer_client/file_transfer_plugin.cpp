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
        root_widget_->hide();

        file_trans_interface_ = std::make_shared<FileTransInterface>(this);
        auto layout = new NoMarginHLayout();
        layout->addWidget(file_trans_interface_->GetFileTransWidget());
        root_widget_->setLayout(layout);

        // title
        root_widget_->setWindowTitle(QString::fromStdString(std::format("{}[{}]", tcTr("id_file_transfer").toStdString(), plugin_settings_.stream_name_)));

        // callbacks
        // upload begin callback
        file_trans_interface_->SetOnFileUploadBeginCallback([=, this](const std::string& task_id, const std::string& file_path) {
            auto event = std::make_shared<ClientPluginFileTransferBeginEvent>();
            event->task_id_ = task_id;
            event->file_path_ = file_path;
            event->direction_ = "Out";
            CallbackEvent(event);
        });

        // upload end callback
        file_trans_interface_->SetOnFileUploadEndCallback([=, this](const std::string& task_id, const std::string& file_path, bool success) {
            auto event = std::make_shared<ClientPluginFileTransferEndEvent>();
            event->task_id_ = task_id;
            event->file_path_ = file_path;
            event->direction_ = "Out";
            event->success_ = success;
            CallbackEvent(event);
        });

        // download begin callback
        file_trans_interface_->SetOnFileDownloadBeginCallback([=, this](const std::string& task_id, const std::string& remote_file_path) {
            auto event = std::make_shared<ClientPluginFileTransferBeginEvent>();
            event->task_id_ = task_id;
            event->file_path_ = remote_file_path;
            event->direction_ = "In";
            CallbackEvent(event);
        });

        // download end callback
        file_trans_interface_->SetOnFileDownloadEndCallback([=, this](const std::string& task_id, const std::string& remote_file_path, bool success) {
            auto event = std::make_shared<ClientPluginFileTransferEndEvent>();
            event->task_id_ = task_id;
            event->file_path_ = remote_file_path;
            event->direction_ = "In";
            event->success_ = success;
            CallbackEvent(event);
        });
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
    }

    void FileTransferPlugin::ShowRootWidget() {
        ClientPluginInterface::ShowRootWidget();
        if (file_trans_interface_) {
            file_trans_interface_->ShowFileTrans();
        }
    }

    void FileTransferPlugin::HideRootWidget() {
        ClientPluginInterface::HideRootWidget();
    }

    bool FileTransferPlugin::HasProcessingTasks() {
        return file_trans_interface_ && file_trans_interface_->HasTransTask();
    }

}
