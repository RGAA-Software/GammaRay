//
// Created RGAA on 15/11/2024.
//

#include "clipboard_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "render/plugins/plugin_ids.h"
#include "tc_opus_codec_new/opus_codec.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/file.h"
#include "tc_common_new/time_util.h"
#include "clipboard_manager.h"
#include "tc_message.pb.h"
#include "win/cp_virtual_file.h"
#include "plugin_interface/gr_plugin_context.h"
#include "tc_message_new/proto_converter.h"

void* GetInstance() {
    static tc::ClipboardPlugin plugin;
    return (void*)&plugin;
}

namespace tc
{
    std::string ClipboardPlugin::GetPluginId() {
        return kClipboardPluginId;
    }

    std::string ClipboardPlugin::GetPluginName() {
        return "Clipboard";
    }

    std::string ClipboardPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t ClipboardPlugin::GetVersionCode() {
        return 110;
    }

    std::string ClipboardPlugin::GetPluginDescription() {
        return "Share clipboard between devices";
    }

    void ClipboardPlugin::On1Second() {

    }

    bool ClipboardPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        clipboard_mgr_ = std::make_shared<ClipboardManager>(this);

        ::OleInitialize(nullptr);
        return true;
    }

    bool ClipboardPlugin::OnDestroy() {
        GrPluginInterface::OnDestroy();
        ::OleUninitialize();
        return true;
    }

    void ClipboardPlugin::OnMessage(const std::shared_ptr<Message>& msg) {
        if (msg->type() == MessageType::kClipboardInfo) {
            if (msg->clipboard_info().type() == ClipboardType::kClipboardText && clipboard_mgr_) {
                plugin_context_->PostUITask([=, this]() {
                    clipboard_mgr_->OnRemoteClipboardInfo(msg);
                });
            }
            if (msg->clipboard_info().type() == ClipboardType::kClipboardFiles) {
                const auto& files = msg->clipboard_info().files();
                std::vector<ClipboardFile> target_files;
                for (auto& file : files) {
                    ClipboardFile cpy_file;
                    cpy_file.CopyFrom(file);
                    target_files.push_back(file);
                }

                this->PostUITask([=, this]() {
                    if (!virtual_file_) {
                        virtual_file_ = tc::CreateVirtualFile(IID_IDataObject, (void **) &data_object_, this);
                    }
                    if (!data_object_) {
                        LOGE("DataObject is null!");
                        return;
                    }

                    bool cleared_clipboard = false;
                    for (int i = 0; i < 100; i++) {
                        auto hr = ::OleSetClipboard(nullptr);
                        if (hr == S_OK) {
                            cleared_clipboard = true;
                            break;
                        }
                        TimeUtil::DelayBySleep(10);
                    }
                    if (!cleared_clipboard) {
                        LOGE("Empty clipboard failed!");
                        return;
                    }

                    TimeUtil::DelayBySleep(10);

                    bool set_clipboard = false;
                    for (int i = 0; i < 100; i++) {
                        auto hr = ::OleSetClipboard(data_object_);
                        if (hr == S_OK) {
                            set_clipboard = true;
                            break;
                        }
                    }
                    if (!set_clipboard) {
                        LOGE("Set clipboard failed!");
                        return;
                    }

                    LOGI("Data obj ref count: {}", virtual_file_->GetRefCount());
                    auto device_id = msg->device_id();
                    auto stream_id = msg->stream_id();
                    virtual_file_->OnClipboardFilesInfo(device_id, stream_id, target_files);
                });
            }
        }
        else if (msg->type() == MessageType::kClipboardInfoResp) {
            // tell the panel, remote info
            auto event = std::make_shared<GrPluginRemoteClipboardResp>();
            auto sub = msg->clipboard_info_resp();
            event->content_type_ = (int)sub.type();
            event->remote_info_ = sub.msg();
            CallbackEvent(event);
            LOGI("received clipboard resp: {}", sub.msg());
        }
        else if (msg->type() == MessageType::kClipboardRespBuffer) {
            if (virtual_file_) {
                virtual_file_->OnClipboardRespBuffer(msg->cp_resp_buffer());
            }
        }
        else if (msg->type() == MessageType::kClipboardReqBuffer) {
            this->OnRequestFileBuffer(msg);
        }
    }

    void ClipboardPlugin::DispatchAppEvent(const std::shared_ptr<AppBaseEvent>& event) {
        if (event->type_ == AppBaseEvent::EType::kClipboardEvent) {
            if (auto ev = std::dynamic_pointer_cast<MsgClipboardEvent>(event); ev) {
                plugin_context_->PostUITask([=, this]() {
                    clipboard_mgr_->OnLocalClipboardUpdated(ev);
                });
            }
        }
    }

    void ClipboardPlugin::OnRequestFileBuffer(std::shared_ptr<Message> in_msg) {
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
        msg.set_device_id(sys_settings_.device_id_);
        msg.set_stream_id(in_msg->stream_id());
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
        auto mb = ProtoAsData(&msg);
        this->DispatchTargetFileTransferMessage(in_msg->stream_id(), mb);
        //LOGI("Req, index: {}, start: {}, size: {}, read size: {}", req_index, req_start, req_size, data ? data->Size() : 0);
    }
}
