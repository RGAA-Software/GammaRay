//
// Created by RGAA on 15/11/2024.
//

#include "plugin_manager.h"
#include "tc_common_new/log.h"
#include <QDir>
#include <QFile>
#include <QApplication>
#include "toml/toml.hpp"
#include "plugin_interface/gr_plugin_interface.h"
#include "plugin_interface/gr_video_encoder_plugin.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_net_plugin.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"
#include "plugin_event_router.h"
#include "plugin_ids.h"
#include "rd_context.h"
#include "rd_app.h"
#include "settings/rd_settings.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    std::shared_ptr<PluginManager> PluginManager::Make(const std::shared_ptr<RdApplication>& app) {
        return std::make_shared<PluginManager>(app);
    }

    PluginManager::PluginManager(const std::shared_ptr<RdApplication>& app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        settings_ = RdSettings::Instance();
    }

    void PluginManager::LoadAllPlugins() {
        auto base_path = QCoreApplication::applicationDirPath();
        LOGI("plugin base path: {}", base_path.toStdString());
        QDir plugin_dir(base_path + R"(/gr_plugins)");
        QStringList filters;
        filters << QString("*%1").arg(".dll");
        plugin_dir.setNameFilters(filters);

        auto entryInfoList = plugin_dir.entryInfoList();
        for (const auto &info: entryInfoList) {
            auto target_plugin_path = base_path + R"(/gr_plugins/)" + info.fileName();
            LOGI("Will load: {}", target_plugin_path.toStdString());

            HMODULE module = LoadLibraryW(target_plugin_path.toStdWString().c_str());
            auto fn_get_instance = GetProcAddress(module, "GetInstance");

            auto func = (FnGetInstance) fn_get_instance;
            if (func) {
                auto plugin = (GrPluginInterface*)func();
                if (plugin) {
                    auto plugin_id = plugin->GetPluginId();
                    if (plugins_.contains(plugin_id)) {
                        LOGE("{} repeated loading.", plugin_id);
                        continue;
                    }

                    // create it
                    auto filename = info.fileName();
                    auto param = GrPluginParam {
                        .cluster_ = {
                            {"name", filename.toStdString()},
                            {"base_path", base_path.toStdString()},
                            {"capture_audio_device_id", settings_->capture_.capture_audio_device_},
                            {"ws-listen-port", (int64_t)settings_->transmission_.listening_port_},
                            {"udp-listen-port", (int64_t)settings_->transmission_.udp_listen_port_},
                            {"device_id", settings_->device_id_},
                            {"relay_host", settings_->relay_host_},
                            {"relay_port", settings_->relay_port_}
                        },
                    };

                    auto config_filepath = plugin_dir.path() + "/" + filename + ".toml";
                    if (QFile::exists(config_filepath)) {
                        try {
                            auto cfg = toml::parse_file(config_filepath.toStdString());
                            cfg.for_each([&](auto& k, auto& v) {
                                auto str_key = (std::string)k;
                                if constexpr (toml::is_string<decltype(v)>) {
                                    auto str_value = toml::value<std::string>(v).get();
                                    param.cluster_.insert({str_key, str_value});
                                }
                                else if constexpr (toml::is_boolean<decltype(v)>) {
                                    auto bool_value = toml::value<bool>(v).get();
                                    param.cluster_.insert({str_key, bool_value});
                                }
                                else if constexpr (toml::is_integer<decltype(v)>) {
                                    auto int_value = toml::value<int64_t>(v).get();
                                    param.cluster_.insert({str_key, int_value});
                                }
                                else if constexpr (toml::is_floating_point<decltype(v)>) {
                                    auto float_value = toml::value<double>(v).get();
                                    param.cluster_.insert({str_key, float_value});
                                }
                            });
                        } catch (const std::exception& e) {
                            LOGE("Parse config: {} failed!", config_filepath.toStdString());
                        }
                    } else {
                        LOGW("The config: {} is not exist!", config_filepath.toStdString());
                        continue;
                    }

                    if (!plugin->OnCreate(param)) {
                        LOGE("Plugin: {} OnCreate failed!", plugin->GetPluginName());
                        continue;
                    }

                    if (!plugin->IsPluginEnabled()) {
                        LOGW("Plugin: {} is disabled!", plugin->GetPluginName());
                        //continue;
                    }

                    plugins_.insert({plugin_id, plugin});

                    LOGI("{} loaded, version: {}", plugin->GetPluginName(), plugin->GetVersionName());
                } else {
                    LOGE("{} object create failed.", info.fileName().toStdString().c_str());
                }
            } else {
                LOGE("{} cannot find symbol.", info.fileName().toStdString().c_str());
            }
        }

        VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            if (plugin->GetPluginType() != GrPluginType::kNet) {
                VisitNetPlugins([=, this](GrNetPlugin* np) {
                    plugin->AttachNetPlugin(np->GetPluginId(), np);
                });
            }
        });
    }

    void PluginManager::RegisterPluginEventsCallback() {
        this->evt_router_ = std::make_shared<PluginEventRouter>(app_);
        VisitAllPlugins([&](GrPluginInterface* plugin) {
            plugin->RegisterEventCallback([=, this](const std::shared_ptr<GrPluginBaseEvent>& event) {
                evt_router_->ProcessPluginEvent(event);
            });
        });
    }

    void PluginManager::ReleaseAllPlugins() {
        for (const auto& [k, plugin] : plugins_) {
            plugin->OnStop();
            plugin->OnDestroy();
        }
        plugins_.clear();
    }

    void PluginManager::ReleasePlugin(const std::string &name) {

    }

    GrPluginInterface* PluginManager::GetPluginById(const std::string& id) {
        if (!plugins_.contains(id)) {
            return nullptr;
        }
        return plugins_.at(id);
    }

    GrVideoEncoderPlugin* PluginManager::GetFFmpegEncoderPlugin() {
        auto plugin = GetPluginById(kFFmpegEncoderPluginId);
        if (plugin) {
            return (GrVideoEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    GrVideoEncoderPlugin* PluginManager::GetNvencEncoderPlugin() {
        auto plugin = GetPluginById(kNvencEncoderPluginId);
        if (plugin) {
            return (GrVideoEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    GrVideoEncoderPlugin* PluginManager::GetAmfEncoderPlugin() {
        auto plugin = GetPluginById(kAmfEncoderPluginId);
        if (plugin) {
            return (GrVideoEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    GrMonitorCapturePlugin* PluginManager::GetDDACapturePlugin() {
        auto plugin = GetPluginById(kDdaCapturePluginId);
        if (plugin) {
            return (GrMonitorCapturePlugin*)plugin;
        }
        return nullptr;
    }

    GrDataProviderPlugin* PluginManager::GetMockVideoStreamPlugin() {
        auto plugin = GetPluginById(kMockVideoStreamPluginId);
        if (plugin) {
            return (GrDataProviderPlugin*)plugin;
        }
        return nullptr;
    }

    GrDataProviderPlugin* PluginManager::GetAudioCapturePlugin() {
        auto plugin = GetPluginById(kWasAudioCapturePluginId);
        if (plugin) {
            return (GrDataProviderPlugin*)plugin;
        }
        return nullptr;
    }

    GrAudioEncoderPlugin* PluginManager::GetAudioEncoderPlugin() {
        auto plugin = GetPluginById(kOpusEncoderPluginId);
        if (plugin) {
            return (GrAudioEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    GrPluginInterface* PluginManager::GetFileTransferPlugin() {
        auto plugin = GetPluginById(kNetFileTransferPluginId);
        if (plugin) {
            return (GrPluginInterface*)plugin;
        }
        return nullptr;
    }

    GrNetPlugin* PluginManager::GetUdpPlugin() {
        auto plugin = GetPluginById(kNetUdpPluginId);
        if (plugin) {
            return (GrNetPlugin*)plugin;
        }
        return nullptr;
    }

    GrPluginInterface* PluginManager::GetClipboardPlugin() {
        auto plugin = GetPluginById(kClipboardPluginId);
        if (plugin) {
            return (GrPluginInterface*)plugin;
        }
        return nullptr;
    }

    GrPluginInterface* PluginManager::GetRtcPlugin() {
        auto plugin = GetPluginById(kNetRtcPluginId);
        if (plugin) {
            return plugin;
        }
        return nullptr;
    }

    void PluginManager::VisitAllPlugins(const std::function<void(GrPluginInterface *)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (visitor) {
                visitor(plugin);
            }
        }
    }

    void PluginManager::VisitStreamPlugins(const std::function<void(GrStreamPlugin *)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (plugin->GetPluginType() == GrPluginType::kStream) {
                visitor((GrStreamPlugin *) plugin);
            }
        }
    }

    void PluginManager::VisitUtilPlugins(const std::function<void(GrPluginInterface *)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (plugin->GetPluginType() == GrPluginType::kUtil) {
                visitor(plugin);
            }
        }
    }

    void PluginManager::VisitEncoderPlugins(const std::function<void(GrVideoEncoderPlugin*)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (plugin->GetPluginType() == GrPluginType::kEncoder) {
                visitor((GrVideoEncoderPlugin *) plugin);
            }
        }
    }

    void PluginManager::VisitNetPlugins(const std::function<void(GrNetPlugin*)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (plugin->GetPluginType() == GrPluginType::kNet) {
                visitor((GrNetPlugin*) plugin);
            }
        }
    }

    void PluginManager::On1Second() {
        VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->On1Second();
        });
    }

    void PluginManager::DumpPluginInfo() {
        LOGI("====> Total plugins: {}", plugins_.size());
        int index = 1;
        VisitAllPlugins([&](GrPluginInterface *plugin) {
            LOGI("Plugin {}. [{}] vn: [{}], vc: [{}], enabled: [{}]",
                 index++,
                 plugin->GetPluginName(),
                 plugin->GetVersionName(),
                 plugin->GetVersionCode(),
                 plugin->IsPluginEnabled()
            );
        });
    }

    void PluginManager::SyncSystemInfo(const GrPluginSettingsInfo& info) {
        VisitAllPlugins([&](GrPluginInterface* plugin) {
            plugin->OnSyncSystemSettings(info);
        });
    }

    int64_t PluginManager::GetQueuingMediaMsgCountInNetPlugins() {
        int64_t queuing_msg_count = 0;
        VisitNetPlugins([&](GrNetPlugin* plugin) {
            if (plugin->ConnectedClientSize() > 0) {
                queuing_msg_count += plugin->GetQueuingMediaMsgCount();
            }
        });
        return queuing_msg_count;
    }

    int64_t PluginManager::GetQueuingFtMsgCountInNetPlugins() {
        int64_t queuing_msg_count = 0;
        VisitNetPlugins([&](GrNetPlugin* plugin) {
            if (plugin->ConnectedClientSize() > 0) {
                queuing_msg_count += plugin->GetQueuingFtMsgCount();
            }
        });
        return queuing_msg_count;
    }

}