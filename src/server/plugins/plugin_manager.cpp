//
// Created by RGAA on 15/11/2024.
//

#include "plugin_manager.h"
#include "tc_common_new/log.h"
#include <QDir>
#include <QFile>
#include <QApplication>
#include "plugin_interface/gr_plugin_interface.h"
#include "plugin_event_router.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    std::shared_ptr<PluginManager> PluginManager::Make(const std::shared_ptr<Context> &ctx) {
        return std::make_shared<PluginManager>(ctx);
    }

    PluginManager::PluginManager(const std::shared_ptr<Context> &ctx) {
        this->context_ = ctx;
        this->evt_router_ = std::make_shared<PluginEventRouter>(ctx);
    }

    void PluginManager::LoadAllPlugins() {
        QDir pluginDir(QCoreApplication::applicationDirPath() + R"(/gr_plugins)");
        QStringList filters;
        filters << QString("*%1").arg(".dll");
        pluginDir.setNameFilters(filters);

        auto entryInfoList = pluginDir.entryInfoList();
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
                        if (plugins_.HasKey(plugin_name)) {
                            LOGE("{} repeated loading.", plugin_name);
                            lib->unload();
                            lib->deleteLater();
                            lib = nullptr;
                            continue;
                        }

                        // create it
                        // todo: load config
                        auto param = GrPluginParam {
                            .cluster_ = {
                                {"name", info.fileName().toStdString()},
                            },
                        };
                        plugin->OnCreate(param);

                        plugins_.Insert(plugin_name, plugin);
                        libs_.insert({plugin_name, lib});

                        LOGI("{} version: {}", plugin->GetPluginName(), plugin->GetVersionName());
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
        VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->RegisterEventCallback([=, this](const std::shared_ptr<GrPluginBaseEvent>& event) {
                evt_router_->ProcessPluginEvent(event);
            });
        });
    }

    void PluginManager::ReleaseAllPlugins() {
        plugins_.ApplyAll([=](auto k, GrPluginInterface* plugin) {
            plugin->OnStop();
            plugin->OnDestroy();
        });
        for (auto &[k, lib]: libs_) {
            lib->unload();
            lib->deleteLater();
        }
        plugins_.Clear();
        libs_.clear();
    }

    void PluginManager::ReleasePlugin(const std::string &name) {

    }

    GrPluginInterface *PluginManager::GetPluginByName(const std::string &name) {
        return plugins_.Get(name);
    }

    void PluginManager::VisitAllPlugins(const std::function<void(GrPluginInterface *)>&& visitor) {
        plugins_.ApplyAll([=](auto k, GrPluginInterface* plugin) {
            if (visitor && plugin->IsPluginEnabled()) {
                visitor(plugin);
            }
        });
    }

    void PluginManager::VisitStreamPlugins(const std::function<void(GrStreamPlugin *)>&& visitor) {
        plugins_.ApplyAll([=](auto k, GrPluginInterface* plugin) {
            if (plugin->GetPluginType() == GrPluginType::kStream) {
                visitor((GrStreamPlugin*)plugin);
            }
        });
    }

    void PluginManager::VisitUtilPlugins(const std::function<void(GrPluginInterface *)>&& visitor) {
        plugins_.ApplyAll([=](auto k, GrPluginInterface* plugin) {
            if (plugin->GetPluginType() == GrPluginType::kUtil) {
                visitor(plugin);
            }
        });
    }

    void PluginManager::VisitEncoderPlugins(const std::function<void(GrEncoderPlugin*)>&& visitor) {
        plugins_.ApplyAll([=](auto k, GrPluginInterface* plugin) {
            if (plugin->GetPluginType() == GrPluginType::kEncoder) {
                visitor((GrEncoderPlugin*)plugin);
            }
        });
    }

    void PluginManager::On1Second() {
        VisitAllPlugins([=, this](GrPluginInterface* plugin) {
            plugin->On1Second();
        });
    }

    void PluginManager::DumpPluginInfo() {
        LOGI("====> Total plugins: {}", plugins_.Size());
        int index = 1;
        VisitAllPlugins([&](GrPluginInterface *plugin) {
            LOGI("Plugin {}. {} Version name:{}, Version code: {}", index++, plugin->GetPluginName(),
                 plugin->GetVersionName(), plugin->GetVersionCode());
        });
    }

}