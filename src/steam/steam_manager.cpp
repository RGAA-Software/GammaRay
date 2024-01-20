//
// Created by hy on 2024/1/17.
//

#include "steam_manager.h"
#include "tc_common/log.h"

#include <Windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

namespace tc
{

    std::shared_ptr<SteamManager> SteamManager::Make(const std::shared_ptr<Context> &ctx) {
        return std::make_shared<SteamManager>(ctx);
    }

    SteamManager::SteamManager(const std::shared_ptr<Context> &ctx) {
        context_ = ctx;
    }

    SteamManager::~SteamManager() {}

    void SteamManager::QueryInstalledApps(HKEY hKey) {
        TCHAR achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD cbName = 0;                   // size of name string
        TCHAR achClass[MAX_PATH] = TEXT("");  // buffer for class name
        DWORD cchClassName = MAX_PATH;  // size of class string
        DWORD cSubKeys = 0;               // number of subkeys
        DWORD cbMaxSubKey = 0;              // longest subkey size
        DWORD cchMaxClass = 0;              // longest class string
        DWORD cValues = 0;              // number of values for key
        DWORD cchMaxValue = 0;          // longest value name
        DWORD cbMaxValueData = 0;       // longest value data
        DWORD cbSecurityDescriptor = 0; // size of security descriptor
        FILETIME ftLastWriteTime;      // last write time

        DWORD i = 0, j = 0, retCode = 0;

        TCHAR achValue[MAX_VALUE_NAME] = {'\0'};
        DWORD cchValue = MAX_VALUE_NAME;

        // Get the class name and the value count.
        retCode = ::RegQueryInfoKey(
                hKey,                    // key handle
                achClass,                // buffer for class name
                &cchClassName,           // size of class string
                NULL,                    // reserved
                &cSubKeys,               // number of subkeys
                &cbMaxSubKey,            // longest subkey size
                &cchMaxClass,            // longest class string
                &cValues,                // number of values for this key
                &cchMaxValue,            // longest value name
                &cbMaxValueData,         // longest value data
                &cbSecurityDescriptor,   // security descriptor
                &ftLastWriteTime);       // last write time
        if (ERROR_SUCCESS != retCode) {
            LOGE("RegQueryInfoKey failed.");
            return;
        }

        // Enumerate the subkeys, until RegEnumKeyEx fails.
        if (cSubKeys) {
            printf("\nNumber of subkeys: %d\n", cSubKeys);

            for (i = 0; i < cSubKeys; i++) {
                cbName = MAX_KEY_LENGTH;
                retCode = ::RegEnumKeyEx(hKey, i,
                                         achKey,
                                         &cbName,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &ftLastWriteTime);

                if (retCode == ERROR_SUCCESS) {

                    Game game;
                    game.app_id_ = QString::fromStdWString(std::wstring(achKey)).toInt();

                    std::wstring name;
                    auto innerSubKey = steam_app_base_path_ + L"\\" + std::wstring(achKey);
                    HKEY innerKey;
                    std::wcout << "app path: " << innerSubKey << std::endl;
                    if (RegOpenKeyExW(HKEY_CURRENT_USER, innerSubKey.c_str(), 0, KEY_READ, &innerKey) == ERROR_SUCCESS) {
                        {
                            wchar_t nameValue[MAX_PATH] = L"Name";
                            DWORD bufferSize = MAX_PATH * sizeof(wchar_t);
                            wchar_t buffer[MAX_PATH] = {0};
                            if (RegGetValueW(innerKey, nullptr, nameValue, RRF_RT_REG_SZ, nullptr, buffer,
                                             &bufferSize) == ERROR_SUCCESS) {
                                std::wcout << "Game name: " << buffer << std::endl;
                                game.name_ = QString::fromStdWString(buffer);
                            }
                        }

                        {
                            wchar_t nameValue[MAX_PATH] = L"Installed";
                            DWORD data = 0;
                            DWORD dataSize = sizeof(DWORD);
                            if (RegGetValueW(innerKey, nullptr, nameValue, RRF_RT_REG_DWORD, nullptr, &data,
                                             &dataSize) == ERROR_SUCCESS) {
                                std::wcout << "Installed : " << data << std::endl;
                                game.installed_ = data;
                            }
                        }

                        {
                            wchar_t nameValue[MAX_PATH] = L"Running";
                            DWORD data = 0;
                            DWORD dataSize = sizeof(DWORD);
                            if (RegGetValueW(innerKey, nullptr, nameValue, RRF_RT_REG_DWORD, nullptr, &data,
                                             &dataSize) == ERROR_SUCCESS) {
                                std::wcout << "Running : " << data << std::endl;
                                game.running_ = data;
                            }
                        }

                        {
                            wchar_t nameValue[MAX_PATH] = L"Updating";
                            DWORD data = 0;
                            DWORD dataSize = sizeof(DWORD);
                            if (RegGetValueW(innerKey, nullptr, nameValue, RRF_RT_REG_DWORD, nullptr, &data,
                                             &dataSize) == ERROR_SUCCESS) {
                                std::wcout << "updating : " << data << std::endl;
                                game.updating_ = data;
                            }
                        }
                        RegCloseKey(innerKey);

                        if (!game.name_.isEmpty() && game.installed_) {
                            games_.push_back(game);
                        }
                    }
                }
            }
        }

        // Enumerate the key values.

        if (cValues) {
            printf("\nNumber of values: %d\n", cValues);

            for (i = 0; i < cValues; i++) {
                cchValue = MAX_VALUE_NAME;
                achValue[0] = '\0';
                retCode = ::RegEnumValue(hKey, i,
                                         achValue,
                                         &cchValue,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);

                if (retCode == ERROR_SUCCESS) {
                    printf(("(%d) %s\n"), i + 1, achValue);
                }
            }
        }
    }

    bool SteamManager::ScanInstalledGames() {
        installed_steam_path_ = ScanInstalledSteamPath();
        steam_app_base_path_ = L"SOFTWARE\\Valve\\Steam\\Apps";

        HKEY hkey;
        if (::RegOpenKeyEx(HKEY_CURRENT_USER, steam_app_base_path_.c_str(), 0, KEY_READ, &hkey) ==
            ERROR_SUCCESS) {
            QueryInstalledApps(hkey);
        }
        ::RegCloseKey(hkey);

        LOGI("steam path: {}", installed_steam_path_.toStdString());
        return true;
    }

    QString SteamManager::ScanInstalledSteamPath() {
#if WIN32
        HKEY hKey;
        LPCWSTR subKey = L"SOFTWARE\\Valve\\Steam";
        LPCWSTR valueName = L"SteamPath";
        DWORD bufferSize = MAX_PATH * sizeof(wchar_t);
        wchar_t buffer[MAX_PATH] = {0};

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

    std::vector<Game> SteamManager::GetInstalledGames() {
        return games_;
    }

    void SteamManager::DumpGamesInfo() {
        LOGI("Total games: {}", games_.size());
        for (auto& game : games_) {
            LOGI("Game: {}", game.Dump());
        }
    }

}