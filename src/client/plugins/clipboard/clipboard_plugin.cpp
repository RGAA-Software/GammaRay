//
// Created RGAA on 15/11/2024.
//

#include "clipboard_plugin.h"
#include "tc_message.pb.h"
#include "tc_common_new/log.h"
#include "tc_common_new/file.h"
#include "tc_common_new/image.h"
#include "client/plugins/ct_plugin_ids.h"
#include "client/plugins/ct_plugin_events.h"
#include "client/plugins/ct_app_events.h"
#include "ct_clipboard_manager.h"
#include "plugin_interface/ct_plugin_context.h"
#include "tc_common_new/md5.h"
#include "tc_message_new/proto_converter.h"

namespace tc
{

    std::string ClientClipboardPlugin::GetPluginId() {
        return kClientClipboardPluginId;
    }

    std::string ClientClipboardPlugin::GetPluginName() {
        return "Clipboard";
    }

    std::string ClientClipboardPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t ClientClipboardPlugin::GetVersionCode() {
        return 110;
    }

    void ClientClipboardPlugin::On1Second() {
        ClientPluginInterface::On1Second();
    }
    
    bool ClientClipboardPlugin::OnCreate(const tc::ClientPluginParam& param) {
        ClientPluginInterface::OnCreate(param);
        plugin_type_ = ClientPluginType::kUtil;

        if (!IsPluginEnabled()) {
            return true;
        }
        root_widget_->hide();

        clipboard_mgr_ = std::make_shared<ClipboardManager>(this);
        clipboard_mgr_->Start();

        return true;
    }

    void ClientClipboardPlugin::OnMessage(std::shared_ptr<Message> msg) {
        ClientPluginInterface::OnMessage(msg);
        if (msg->type() == MessageType::kClipboardInfo) {
            // server -> text message -> client
            plugin_context_->PostUITask([=, this]() {
                if (clipboard_mgr_) {
                    clipboard_mgr_->OnRemoteClipboardMessage(msg);
                }
            });
        }
        else if (msg->type() == tc::kClipboardInfoResp) {
            plugin_context_->PostUITask([=, this]() {
                if (clipboard_mgr_) {
                    clipboard_mgr_->OnRemoteClipboardRespMessage(msg);
                }
            });
        }
        else if (msg->type() == tc::kClipboardReqAtBegin) {
            // begin; server -> client
            // copy files from client -> server
            plugin_context_->PostWorkTask([=, this]() {
                this->OnRequestFileBegin(msg);
            });
        }
        else if (msg->type() == tc::kClipboardReqBuffer) {
            // transferring
            // server -> request a part of data in the file -> client -> response -> server
            plugin_context_->PostWorkTask([=, this]() {
                this->OnRequestFileBuffer(msg);
            });
        }
        else if (msg->type() == tc::kClipboardReqAtEnd) {
            // end; server -> client
            // copy files from client -> server
            plugin_context_->PostWorkTask([=, this]() {
                this->OnRequestFileEnd(msg);
            });
        }
        else if (msg->type() == MessageType::kClipboardRespBuffer) {
            // server -> response a part of data in the file -> client
            plugin_context_->PostWorkTask([=, this]() {
                if (clipboard_mgr_) {
                    clipboard_mgr_->OnRemoteFileRespMessage(msg);
                }
            });
        }
    }

    void ClientClipboardPlugin::DispatchAppEvent(const std::shared_ptr<ClientAppBaseEvent> &event) {
        ClientPluginInterface::DispatchAppEvent(event);
    }

    void ClientClipboardPlugin::OnLocalClipboardUpdated() {
        plugin_context_->PostUITask([this]() {
            clipboard_mgr_->OnLocalClipboardUpdated();
        });
    }

    bool ClientClipboardPlugin::IsClipboardEnabled() {
        return plugin_settings_.clipboard_enabled_;
    }

    void ClientClipboardPlugin::OnRequestFileBegin(const std::shared_ptr<Message>& msg) {
        auto sub = msg->cp_req_at_begin();
        auto event = std::make_shared<ClientPluginFileTransferBeginEvent>();
        event->task_id_ = MD5::Hex(sub.full_name());
        event->file_path_ = sub.full_name();
        event->direction_ = "Out";
        CallbackEvent(event);
    }

    void ClientClipboardPlugin::OnRequestFileBuffer(const std::shared_ptr<Message>& in_msg) {
        const auto& buffer = in_msg->cp_req_buffer();
        auto req_index = buffer.req_index();
        auto req_start = buffer.req_start();
        auto req_size = buffer.req_size();
        auto full_filename = buffer.full_name();

        auto file = File::OpenForReadB(full_filename);
        DataPtr data = nullptr;
        if (file->Exists()) {
            uint64_t read_size = 0;
            data = file->Read(req_start, req_size, read_size);
        }

        tc::Message msg;
        msg.set_device_id(plugin_settings_.device_id_);
        msg.set_stream_id(plugin_settings_.stream_id_);
        msg.set_type(MessageType::kClipboardRespBuffer);
        auto sub = msg.mutable_cp_resp_buffer();
        sub->set_full_name(full_filename);
        sub->set_req_size(req_size);
        sub->set_req_start(req_start);
        sub->set_req_index(req_index);
        if (data) {
            sub->set_read_size(data->Size());
            sub->set_buffer(data->AsString());
        }
        //LOGI("Req, index: {}, start: {}, size: {}, read size: {}", req_index, req_start, req_size, data ? data->Size() : 0);
        auto event = std::make_shared<ClientPluginNetworkEvent>();
        event->media_channel_ = false;
        event->buf_ = tc::ProtoAsData(&msg);
        CallbackEvent(event);
    }

    void ClientClipboardPlugin::OnRequestFileEnd(const std::shared_ptr<Message>& msg) {
        auto sub = msg->cp_req_at_end();
        auto event = std::make_shared<ClientPluginFileTransferEndEvent>();
        event->task_id_ = MD5::Hex(sub.full_name());
        event->file_path_ = sub.full_name();
        event->direction_ = "Out";
        event->success_ = sub.success();
        CallbackEvent(event);
    }
}

void* GetInstance() {
    static tc::ClientClipboardPlugin plugin;
    return (void*)&plugin;
}