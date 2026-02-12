//
// Created by RGAA on 13/11/2025.
//

#include "skin_loader.h"
#include <QLibrary>
#include <QApplication>
#include "toml/toml.hpp"
#include "tc_common_new/log.h"
#include "interface/skin_interface.h"

typedef void* (*FnGetInstance)();

namespace tc
{

    SkinInterface* SkinLoader::LoadSkin() {
        std::string skin_name;
#ifdef OPENSOURCE_BUILD
        skin_name = "skin_opensource";
#elif defined(OFFICIAL_BUILD)
        skin_name = "skin_official";
#else
        skin_name = "skin_official";
#endif
        LOGI("Prebuilt skin name: {}", skin_name);

        auto base_path = QCoreApplication::applicationDirPath();
        if (skin_name.empty()) {
            auto config_path = base_path + "/gr_skins/skin_config.toml";

            toml::parse_result result;
            try {
                result = toml::parse_file(config_path.toStdString());
            } catch (std::exception& e) {
                LOGE("Parse skin config failed: {}, skin path: {}", e.what(), config_path.toStdString());
                return nullptr;
            }

            skin_name = result["skin_name"].value_or("");
            if (skin_name.empty()) {
                LOGE("Skin name is empty!");
                return nullptr;
            }
        }

#ifdef WIN32
        auto lib_name = skin_name + ".dll";
#else
        auto lib_name = skin_name + ".so";
#endif
        auto skin_path = base_path + "/gr_skins/" + lib_name.c_str();
        LOGI("Target skin: {}", skin_path.toStdString());
        auto library = new QLibrary(skin_path);
        if (!library->load()) {
            LOGE("Load skin dll failed: {}", skin_path.toStdString());
            return nullptr;
        }
        auto fn_get_instance = (FnGetInstance)library->resolve("GetInstance");
        auto func = (FnGetInstance)fn_get_instance;
        if (!func) {
            LOGE("Don't have GetInstance in: {}", lib_name);
            return nullptr;
        }

        auto skin = (SkinInterface*)func();
        if (!skin) {
            LOGE("Can't exe GetInstance in skin: {}", lib_name);
            return nullptr;
        }

        if (!skin->OnCreate(SkinParam{})) {
            LOGE("Can't init skin: {}", lib_name);
            return nullptr;
        }
        LOGI("Load skin: {} success.", skin_name);
        return skin;
    }

}
