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
#include "file_transfer/file_transfer_plugin.h"
#include "plugin_event_router.h"
#include "plugin_ids.h"
#include "context.h"
#include "app.h"
#include "settings/settings.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    std::shared_ptr<PluginManager> PluginManager::Make(const std::shared_ptr<Application>& app) {
        return std::make_shared<PluginManager>(app);
    }

    PluginManager::PluginManager(const std::shared_ptr<Application>& app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        settings_ = Settings::Instance();
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
            //auto lib = new QLibrary(base_path + R"(/gr_plugins/)" + info.fileName());
            LOGI("Will load: {}", target_plugin_path.toStdString());
            //auto lib = new QLibrary(base_path + R"(/)" + info.fileName());
            //lib->setLoadHints(QLibrary::ResolveAllSymbolsHint);
//            if (lib->isLoaded()) {
//                LOGI("{} is loaded.", info.fileName().toStdString().c_str());
//                continue;
//            }

            HMODULE module = LoadLibraryW(target_plugin_path.toStdWString().c_str());
            auto fn_get_instance = GetProcAddress(module, "GetInstance");
            //if (lib->load()) {
                //auto func = (FnGetInstance) lib->resolve("GetInstance");
                auto func = (FnGetInstance) fn_get_instance;
                if (func) {
                    auto plugin = (GrPluginInterface*)func();
                    if (plugin) {
                        auto plugin_id = plugin->GetPluginId();
                        if (plugins_.contains(plugin_id)) {
                            LOGE("{} repeated loading.", plugin_id);
//                            lib->unload();
//                            lib->deleteLater();
//                            lib = nullptr;
                            continue;
                        }

                        // create it
                        auto filename = info.fileName();
                        auto param = GrPluginParam {
                            .cluster_ = {
                                {"name", filename.toStdString()},
                                {"base_path", base_path.toStdString()},
                                {"capture_monitor_name", settings_->capture_.capture_monitor_},
                                {"capture_audio_device_id", settings_->capture_.capture_audio_device_},
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
                            continue;
                        }

                        plugins_.insert({plugin_id, plugin});
                        //libs_.insert({plugin_id, lib});

                        LOGI("{} loaded, version: {}", plugin->GetPluginName(), plugin->GetVersionName());
                    } else {
                        LOGE("{} object create failed.", info.fileName().toStdString().c_str());
//                        lib->unload();
//                        lib->deleteLater();
//                        lib = nullptr;
                    }
                } else {
                    LOGE("{} cannot find symbol.", info.fileName().toStdString().c_str());
//                    lib->unload();
//                    lib->deleteLater();
//                    lib = nullptr;
                }
//            } else {
//                LOGE("{} load failed. error message: {}", info.fileName().toStdString().c_str(), lib->errorString().toStdString().c_str());
//                lib->deleteLater();
//                lib = nullptr;
//            }
        }

        // file transfer plugin
        auto ft_plugin = GetFileTransferPlugin();

        // attach to
        VisitNetPlugins([=, this](GrNetPlugin* plugin) {
            if (ft_plugin) {
                ft_plugin->AttachNetPlugin(plugin->GetPluginId(), plugin);
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
        for (auto &[k, lib]: libs_) {
            lib->unload();
            lib->deleteLater();
        }
        plugins_.clear();
        libs_.clear();
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

    FileTransferPlugin* PluginManager::GetFileTransferPlugin() {
        auto plugin = GetPluginById(kNetFileTransferPluginId);
        if (plugin) {
            return (FileTransferPlugin*)plugin;
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

}