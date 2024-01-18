//
// Created by hy on 2024/1/17.
//

#include "steam_manager.h"
#include "tc_common/log.h"

#include <Windows.h>

namespace tc
{

    std::shared_ptr<SteamManager> SteamManager::Make(const std::shared_ptr<Context>& ctx) {
        return std::make_shared<SteamManager>(ctx);
    }

    SteamManager::SteamManager(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    SteamManager::~SteamManager() {}

    bool SteamManager::Init() {
        installed_steam_path_ = ScanInstalledSteamPath();

        LOGI("steam path: {}", installed_steam_path_.toStdString());
        return true;
    }

    QString SteamManager::ScanInstalledSteamPath() {
#if WIN32
        HKEY hKey;
        LPCWSTR subKey = L"SOFTWARE\\Valve\\Steam";
        LPCWSTR valueName = L"SteamPath";
        DWORD bufferSize = MAX_PATH * sizeof(wchar_t);
        wchar_t buffer[MAX_PATH] = { 0 };

        if (RegOpenKeyExW(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegGetValueW(hKey, nullptr, valueName, RRF_RT_REG_SZ, nullptr, buffer, &bufferSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return QString::fromStdWString(buffer);
            }
            RegCloseKey(hKey);
        }

        return QString::fromStdWString(L"");
#endif
    }

}