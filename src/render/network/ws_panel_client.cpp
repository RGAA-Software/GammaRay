//
// Created by RGAA on 2024-04-20.
//

#include "ws_panel_client.h"
#include "rd_context.h"
#include "render/app/app_messages.h"
#include "render/rd_statistics.h"
#include "render/settings/rd_settings.h"
#include "tc_common_new/log.h"
#include "tc_common_new/message_notifier.h"
#include "tc_render_panel_message.pb.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_plugin_interface.h"

namespace tc
{

    const int kMaxClientQueuedMessage = 1024;

    WsPanelClient::WsPanelClient(const std::shared_ptr<RdContext>& ctx) {
        statistics_ = RdStatistics::Instance();
        settings_ = RdSettings::Instance();
        context_ = ctx;
        plugin_mgr_ = context_->GetPluginManager();
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgTimer500>([=, this](const MsgTimer500& msg) {
            context_->PostTask([this]() {
                ReportStatistics();
            });
        });

        msg_listener_->Listen<MsgClientConnected>([=, this](const MsgClientConnected& msg) {
            context_->PostTask([this]() {
                ReportStatistics();
            });
        });

        msg_listener_->Listen<MsgClientDisconnected>([=, this](const MsgClientDisconnected& msg) {
            context_->PostTask([this]() {
                ReportStatistics();
            });
        });
    }

    void WsPanelClient::Start() {
        client_ = std::make_shared<asio2::wss_client>();
        client_->set_auto_reconnect(true);
        client_->set_timeout(std::chrono::milliseconds(2000));
        //client_->set_verify_mode(asio::ssl::verify_peer);
        client_->bind_init([&]() {
            client_->ws_stream().binary(true);
            client_->set_no_delay(true);
            client_->ws_stream().set_option(
                websocket::stream_base::decorator([](websocket::request_type &req) {
                    req.set(http::field::authorization, "websocket-client-authorization");}
                )
            );

        }).bind_connect([&]() {
            if (asio2::get_last_error()) {
                LOGE("WsPanelClient,connect failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
            } else {
                LOGI("WsPanelClient,connect success : {} {} ", client_->local_address().c_str(), client_->local_port());
            }
        }).bind_upgrade([&]() {
            if (asio2::get_last_error()) {
                LOGE("WsPanelClient,upgrade failure : {}, {}", asio2::last_error_val(), asio2::last_error_msg());
            }
        }).bind_recv([this](std::string_view data) {
            ParseNetMessage(data);
        });

        LOGI("Will connect to panel : {}", settings_->panel_server_port_);
        if (!client_->async_start("127.0.0.1", settings_->panel_server_port_, "/panel/renderer")) {
            LOGE("connect websocket server failure : {} {}", asio2::last_error_val(), asio2::last_error_msg().c_str());
        }
    }

    void WsPanelClient::Exit() {
        if (client_) {
            client_->stop();
        }
    }

    void WsPanelClient::ReportStatistics() {
        this->SendStatisticsInternal();
        this->SendPluginsInfoInternal();
    }

    void WsPanelClient::SendStatisticsInternal() {
        PostNetMessage(statistics_->AsProtoMessage());
    }

    void WsPanelClient::SendPluginsInfoInternal() {
        tcrp::RpMessage msg;
        msg.set_type(tcrp::kRpPluginsInfo);
        auto m_info = msg.mutable_plugins_info();
        auto plugins_info = m_info->mutable_plugins_info();
        plugin_mgr_->VisitAllPlugins([&](GrPluginInterface* plugin) {
            auto info = plugins_info->Add();
            info->set_id(plugin->GetPluginId());
            info->set_name(plugin->GetPluginName());
            info->set_author(plugin->GetPluginAuthor());
            info->set_desc(plugin->GetPluginDescription());
            info->set_version_name(plugin->GetVersionName());
            info->set_version_code((int32_t)plugin->GetVersionCode());
            info->set_enabled(plugin->IsPluginEnabled());
        });

        PostNetMessage(msg.SerializeAsString());
    }

    void WsPanelClient::PostNetMessage(const std::string& msg) {
        if (client_ && client_->is_started()) {
            if (queuing_message_count_ > kMaxClientQueuedMessage) {
                return;
            }
            queuing_message_count_++;
            client_->async_send(msg, [=, this]() {
                queuing_message_count_--;
            });
        }
    }

    void WsPanelClient::ParseNetMessage(std::string_view _msg) {
        try {
            std::string msg = std::string(_msg);
            tcrp::RpMessage m;
            m.ParseFromString(msg);
            if (m.type() == tcrp::RpMessageType::kSyncPanelInfo) {
                const auto& sub = m.sync_panel_info();
                settings_->device_id_ = sub.device_id();
                settings_->device_random_pwd_ = sub.device_random_pwd();
                settings_->device_safety_pwd_ = sub.device_safety_pwd();
                settings_->relay_host_ = sub.relay_host();
                settings_->relay_port_ = sub.relay_port();
                settings_->can_be_operated_ = sub.can_be_operated();
                settings_->relay_enabled_ = sub.relay_enabled();
                settings_->language_ = sub.language();

                plugin_mgr_->SyncPluginSettingsInfo(GrPluginSettingsInfo {
                    .device_id_ = settings_->device_id_,
                    .device_random_pwd_ = settings_->device_random_pwd_,
                    .device_safety_pwd_ = settings_->device_safety_pwd_,
                    .relay_host_ = settings_->relay_host_,
                    .relay_port_ = settings_->relay_port_,
                    .can_be_operated_ = settings_->can_be_operated_,
                    .relay_enabled_ = settings_->relay_enabled_,
                    .language_ = settings_->language_,
                });
            }
            else if (m.type() == tcrp::RpMessageType::kRpCommandRenderer) {
                LOGI("====> CommandRenderer <====");
                const auto& sub = m.command_renderer();
                int ws_port = sub.ws_port();
                const auto& plugin_id = sub.plugin_id();
                LOGI("Plugin id: {}", plugin_id);
                if (sub.command() == tcrp::RpPanelCommand::kEnablePlugin) {
                    ProcessCommandEnablePlugin(plugin_id);
                }
                else if (sub.command() == tcrp::RpPanelCommand::kDisablePlugin) {
                    ProcessCommandDisablePlugin(plugin_id);
                }
            }
            else if (m.type() == tcrp::RpMessageType::kRpClipboardEvent) {
                const auto& clipboard_info = m.clipboard_info();
                LOGI("*** Clipboard type: {}, text: {}, file size: {}", (int)clipboard_info.type(), clipboard_info.msg(), clipboard_info.files_size());

                auto event = std::make_shared<MsgClipboardEvent>();
                event->clipboard_type_ = [&]() {
                   if (clipboard_info.type() == tcrp::RpClipboardType::kRpClipboardText) {
                       return MsgClipboardType::kText;
                   }
                   else {
                       return MsgClipboardType::kFiles;
                   }
                }();
                event->text_msg_ = clipboard_info.msg();
                for (const auto& file : clipboard_info.files()) {
                    event->files_.push_back(MsgClipboardFile {
                        .file_name_ = file.file_name(),
                        .full_path_ = file.full_path(),
                        .total_size_ = file.total_size(),
                        .ref_path_ = file.ref_path(),
                    });
                }
                context_->DispatchAppEvent2Plugins(event);
            }

        } catch(std::exception& e) {
            LOGE("ParseNetMessage failed: {}", e.what());
        }
    }

    void WsPanelClient::ProcessCommandEnablePlugin(const std::string& plugin_id) {
        plugin_mgr_->VisitAllPlugins([&](GrPluginInterface* plugin) {
            if (plugin_id == plugin->GetPluginId()) {
                LOGI("Enable plugin: {}", plugin->GetPluginName());
                plugin->EnablePlugin();
            }
        });
    }

    void WsPanelClient::ProcessCommandDisablePlugin(const std::string& plugin_id) {
        plugin_mgr_->VisitAllPlugins([&](GrPluginInterface* plugin) {
            if (plugin_id == plugin->GetPluginId()) {
                LOGI("Disable plugin: {}", plugin->GetPluginName());
                plugin->DisablePlugin();
            }
        });
    }

}