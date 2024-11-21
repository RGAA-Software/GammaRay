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
#include "plugin_interface/gr_encoder_plugin.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_event_router.h"
#include "plugins/ffmpeg_encoder/ffmpeg_encoder_defs.h"
#include "plugins/amf_encoder/amf_encoder_defs.h"
#include "context.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    std::shared_ptr<PluginManager> PluginManager::Make(const std::shared_ptr<Context> &ctx) {
        return std::make_shared<PluginManager>(ctx);
    }

    PluginManager::PluginManager(const std::shared_ptr<Context> &ctx) {
        this->context_ = ctx;
    }

    void PluginManager::LoadAllPlugins() {
        QDir plugin_dir(QCoreApplication::applicationDirPath() + R"(/gr_plugins)");
        QStringList filters;
        filters << QString("*%1").arg(".dll");
        plugin_dir.setNameFilters(filters);

        auto entryInfoList = plugin_dir.entryInfoList();
        for (const auto &info: entryInfoList) {
            auto lib = new QLibrary(QCoreApplication::applicationDirPath() + R"(/gr_plugins/)" + info.fileName());
            if (lib->isLoaded()) {
                LOGI("{} is loaded.", info.fileName().toStdString().c_str());
                continue;
            }

            if (lib->load()) {
                auto func = (FnGetInstance) lib->resolve("GetInstance");
                if (func) {
                    auto plugin = (GrPluginInterface *) func();
                    if (plugin) {
                        auto plugin_name = plugin->GetPluginName();
                        if (plugins_.contains(plugin_name)) {
                            LOGE("{} repeated loading.", plugin_name);
                            lib->unload();
                            lib->deleteLater();
                            lib = nullptr;
                            continue;
                        }

                        // create it
                        auto filename = info.fileName();
                        auto param = GrPluginParam {
                            .cluster_ = {
                                {"name", filename.toStdString()},
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
                        }

                        if (!plugin->OnCreate(param)) {
                            LOGE("Plugin: {} OnCreate failed!", plugin->GetPluginName());
                            continue;
                        }

                        plugins_.insert({plugin_name, plugin});
                        libs_.insert({plugin_name, lib});

                        LOGI("{} loaded, version: {}", plugin->GetPluginName(), plugin->GetVersionName());
                    } else {
                        LOGE("{} object create failed.", info.fileName().toStdString().c_str());
                        lib->unload();
                        lib->deleteLater();
                        lib = nullptr;
                    }
                } else {
                    LOGE("{} cannot find symbol.", info.fileName().toStdString().c_str());
                    lib->unload();
                    lib->deleteLater();
                    lib = nullptr;
                }
            } else {
                LOGE("{} load failed. error message: {}", info.fileName().toStdString().c_str(), lib->errorString().toStdString().c_str());
                lib->deleteLater();
                lib = nullptr;
            }
        }
    }

    void PluginManager::RegisterPluginEventsCallback() {
        this->evt_router_ = std::make_shared<PluginEventRouter>(context_);
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

    GrPluginInterface* PluginManager::GetPluginByName(const std::string& name) {
        if (!plugins_.contains(name)) {
            return nullptr;
        }
        return plugins_.at(name);
    }

    GrEncoderPlugin* PluginManager::GetFFmpegEncoderPlugin() {
        auto plugin = GetPluginByName(kFFmpegPluginName);
        if (plugin) {
            return (GrEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    GrEncoderPlugin* PluginManager::GetNvencEncoderPlugin() {
        return nullptr;
    }

    GrEncoderPlugin* PluginManager::GetAmfEncoderPlugin() {
        auto plugin = GetPluginByName(kAmfPluginName);
        if (plugin) {
            return (GrEncoderPlugin*)plugin;
        }
        return nullptr;
    }

    void PluginManager::VisitAllPlugins(const std::function<void(GrPluginInterface *)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (visitor && plugin->IsPluginEnabled()) {
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

    void PluginManager::VisitEncoderPlugins(const std::function<void(GrEncoderPlugin*)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (plugin->GetPluginType() == GrPluginType::kEncoder) {
                visitor((GrEncoderPlugin *) plugin);
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
            LOGI("Plugin {}. [{}] version name: [{}], version code: [{}], enabled: [{}]",
                 index++,
                 plugin->GetPluginName(),
                 plugin->GetVersionName(),
                 plugin->GetVersionCode(),
                 plugin->IsPluginEnabled()
            );
        });
    }

}