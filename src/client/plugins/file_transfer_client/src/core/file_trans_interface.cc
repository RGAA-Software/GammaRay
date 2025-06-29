#include "file_trans_interface.h"
#include <iostream>
#include <qapplication.h>
#include <format>
#include "tc_label.h"
#include "ct_plugin_events.h"
#include "file_sdk_interface.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"
#include "widget/file_log_manager.h"
#include "file_transfer_client/file_transfer_plugin.h"
#include "file_transfer_client/src/widget/file_trans_widget.h"
#include "file_transfer_client/src/widget/file_transmit_single_task_manager.h"

namespace tc
{

// 调用关系 file_trans_interface->file_sdk_interface->file_transmit_sdk

    FileTransInterface::FileTransInterface(FileTransferPlugin* plugin) : plugin_(plugin) {
        auto settings = plugin_->GetPluginSettings();
        auto device_id = settings.device_id_;
        std::string stream_id = settings.stream_id_;
        FileSDKInterface::Instance()->InitFileTransSDK(device_id, stream_id);

        // init language
        tcTrMgr()->InitLanguage((LanguageKind)settings.language_);

        // 先初始化InitFileTransSDK, 再设置回调
        FileSDKInterface::Instance()->RegSendMessageFunc([=, this](std::shared_ptr<tc::Message> msg) {
            std::string msg_str = msg->SerializeAsString();
            this->SendProtoMessage(msg_str);
            return true;
        });
        // widget
        file_trans_widget_ = new FileTransWidget();
        //file_trans_widget_->setWindowTitle(QString::fromStdString(std::format("File Transfer[{}]", settings.stream_name_)));
        file_trans_widget_->SetDevicesId(QString::fromStdString(settings.display_name_), QString::fromStdString(settings.display_remote_name_));
        FileSDKInterface::Instance()->file_trans_widget_ = file_trans_widget_;
    }

    FileTransInterface::~FileTransInterface() {

    }

    void FileTransInterface::OnProtoMessage(const std::shared_ptr<Message>& msg) {
        //
        //LOGI("OnMessage: {}", (int)msg->type());
        FileSDKInterface::Instance()->OnMessage(msg);
    }

    void FileTransInterface::SendProtoMessage(const std::string& msg_str) {
        // TODO:///
        //if (sdk_ptr_) {
        //	sdk_ptr_->PostFileTransferMessage(msg_str);
        //}
        auto event = std::make_shared<ClientPluginNetworkEvent>();
        event->media_channel_ = false;
        event->buf_ = msg_str;
        plugin_->CallbackEvent(event);
    }

    void FileTransInterface::ShowFileTrans() {
        if (!file_trans_widget_) {
            return;
        }
        file_trans_widget_->RefreshByComboboxSelectDirContent();
        file_trans_widget_->raise();
        file_trans_widget_->activateWindow();
        file_trans_widget_->showNormal();
    }

    void FileTransInterface::Exit() {
        if (file_trans_widget_) {
            file_trans_widget_->SaveCurrentVisitPath();
        }
    }

    bool FileTransInterface::HasTransTask() {
        return FileTransmitSingleTaskManager::Instance()->HasTask();
    }

    void FileTransInterface::SetOnFileUploadBeginCallback(OnFileUploadBeginCallback&& cbk) {
        FileSDKInterface::Instance()->SetOnFileUploadBeginCallback(std::move(cbk));
    }

    void FileTransInterface::SetOnFileUploadEndCallback(OnFileUploadEndCallback&& cbk) {
        FileSDKInterface::Instance()->SetOnFileUploadEndCallback(std::move(cbk));
    }

    void FileTransInterface::SetOnFileDownloadBeginCallback(OnFileDownloadBeginCallback&& cbk) {
        FileSDKInterface::Instance()->SetOnFileDownloadBeginCallback(std::move(cbk));
    }

    void FileTransInterface::SetOnFileDownloadEndCallback(OnFileDownloadEndCallback&& cbk) {
        FileSDKInterface::Instance()->SetOnFileDownloadEndCallback(std::move(cbk));
    }

    QWidget* FileTransInterface::GetFileTransWidget() {
        return file_trans_widget_;
    }

}