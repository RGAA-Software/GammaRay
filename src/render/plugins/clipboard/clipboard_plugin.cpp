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
        return "Clipboard Plugin";
    }

    std::string ClipboardPlugin::GetVersionName() {
        return "1.1.0";
    }

    uint32_t ClipboardPlugin::GetVersionCode() {
        return 110;
    }

    void ClipboardPlugin::On1Second() {

    }

    bool ClipboardPlugin::OnCreate(const tc::GrPluginParam &param) {
        GrPluginInterface::OnCreate(param);
        clipboard_mgr_ = std::make_shared<ClipboardManager>(this);
        clipboard_mgr_->Monitor();
        return true;
    }

    bool ClipboardPlugin::OnDestroy() {
        GrPluginInterface::OnDestroy();
        return true;
    }

    void ClipboardPlugin::OnMessage(const std::string &msg) {

    }

    void ClipboardPlugin::OnMessage(const std::shared_ptr<Message>& msg) {
        if (clipboard_mgr_) {
            clipboard_mgr_->UpdateRemoteInfo(msg);
        }
    }
}
