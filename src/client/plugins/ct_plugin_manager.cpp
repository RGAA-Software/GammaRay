//
// Created by RGAA on 15/11/2024.
//

#include "ct_plugin_manager.h"
#include "tc_common_new/log.h"
#include <QDir>
#include <QFile>
#include <QApplication>
#include "toml/toml.hpp"
#include "ct_client_context.h"
#include "ct_plugin_event_router.h"
#include "plugin_interface/ct_plugin_interface.h"
#include "ct_plugin_ids.h"
#include "ct_settings.h"
//#include "media_record/media_record_plugin.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    std::shared_ptr<ClientPluginManager> ClientPluginManager::Make(const std::shared_ptr<ClientContext>& ctx) {
        return std::make_shared<ClientPluginManager>(ctx);
    }

    ClientPluginManager::ClientPluginManager(const std::shared_ptr<ClientContext>& ctx) {
        this->context_ = ctx;
    }

    void ClientPluginManager::LoadAllPlugins() {
        auto base_path = QCoreApplication::applicationDirPath();
        LOGI("plugin base path: {}", base_path.toStdString());
        QDir plugin_dir(base_path + R"(/gr_plugins_client)");
        QStringList filters;
        filters << QString("*%1").arg(".dll");
        plugin_dir.setNameFilters(filters);

        auto entryInfoList = plugin_dir.entryInfoList();
        for (const auto &info: entryInfoList) {
            auto target_plugin_path = base_path + R"(/gr_plugins_client/)" + info.fileName();
            LOGI("Will load: {}", target_plugin_path.toStdString());

            auto library = new QLibrary(target_plugin_path);
            library->load();
            auto fn_get_instance = (FnGetInstance)library->resolve("GetInstance");

            //HMODULE module = LoadLibraryW(target_plugin_path.toStdWString().c_str());
            //auto fn_get_instance = GetProcAddress(module, "GetInstance");

            auto func = (FnGetInstance) fn_get_instance;
            if (func) {
                auto plugin = (ClientPluginInterface*)func();
                if (plugin) {
                    auto plugin_id = plugin->GetPluginId();
                    if (plugins_.contains(plugin_id)) {
                        LOGE("{} repeated loading.", plugin_id);
                        continue;
                    }

                    auto settings = tc::Settings::Instance();

                    // create it
                    auto filename = info.fileName();
                    auto param = ClientPluginParam {
                        .cluster_ = {
                            {"name", filename.toStdString()},
                            {"base_path", base_path.toStdString()},
                            {"screen_recording_path", settings->screen_recording_path_}
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
    }

    void ClientPluginManager::RegisterPluginEventsCallback() {
        this->evt_router_ = std::make_shared<ClientPluginEventRouter>(context_);
        VisitAllPlugins([&](ClientPluginInterface* plugin) {
            plugin->RegisterEventCallback([=, this](const std::shared_ptr<ClientPluginBaseEvent>& event) {
                evt_router_->ProcessPluginEvent(event);
            });
        });
    }

    void ClientPluginManager::ReleaseAllPlugins() {
        for (const auto& [k, plugin] : plugins_) {
            plugin->OnStop();
            plugin->OnDestroy();
        }
        plugins_.clear();
    }

    void ClientPluginManager::ReleasePlugin(const std::string &name) {

    }

    ClientPluginInterface* ClientPluginManager::GetPluginById(const std::string& id) {
        if (!plugins_.contains(id)) {
            return nullptr;
        }
        return plugins_.at(id);
    }

    void ClientPluginManager::VisitAllPlugins(const std::function<void(ClientPluginInterface *)>&& visitor) {
        for (const auto& [k, plugin] : plugins_) {
            if (visitor) {
                visitor(plugin);
            }
        }
    }

    void ClientPluginManager::On1Second() {
        VisitAllPlugins([=, this](ClientPluginInterface* plugin) {
            plugin->On1Second();
        });
    }

    void ClientPluginManager::DumpPluginInfo() {
        LOGI("====> Total plugins: {}", plugins_.size());
        int index = 1;
        VisitAllPlugins([&](ClientPluginInterface *plugin) {
            LOGI("Plugin {}. [{}] vn: [{}], vc: [{}], enabled: [{}]",
                 index++,
                 plugin->GetPluginName(),
                 plugin->GetVersionName(),
                 plugin->GetVersionCode(),
                 plugin->IsPluginEnabled()
            );
        });
    }

    MediaRecordPluginClient* ClientPluginManager::GetMediaRecordPlugin() {
        auto plugin = GetPluginById(kMediaRecordPluginId);
        if (plugin) {
            return (MediaRecordPluginClient*)plugin;
        }
        return nullptr;
    }
}