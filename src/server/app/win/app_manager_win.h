//
// Created by RGAA on 2023-12-21.
//

#ifndef TC_APPLICATION_APP_MANAGER_WIN_H
#define TC_APPLICATION_APP_MANAGER_WIN_H

#include <map>
#include <string>
#include "app/app_manager.h"
#include "tc_common_new/win32/process_helper.h"

namespace tc
{

    class Settings;
    class SteamGame;

    class AppManagerWinImpl : public AppManager {
    public:

        explicit AppManagerWinImpl(const std::shared_ptr<Context>& ctx);
        ~AppManagerWinImpl() override;

        void Init() override;
        bool StartProcessWithHook() override;
        bool StartProcess() override;
        void Exit() override;
        void* GetWindowHandle() override;
        void CloseCurrentApp() override;

    private:
        void InjectCaptureDllIfNeeded();
        void InjectCaptureDllForSteamApp();
        void InjectCaptureDllForNormalApp();
        bool InjectDll(uint32_t pid, uint32_t tid, bool is_x86, const std::string& x86_dll, const std::string& x64_dll);
        void AddFoundPid(const ProcessInfo& target_pi);
        static WindowInfos SearchWindowByPid(uint32_t pid);
        static WindowInfo GetTargetWindowInfo(const WindowInfos& infos);

    private:
        Settings* settings_;
        unsigned long target_pid_ = 0;
        WindowInfo target_window_info_;
        std::atomic<bool> injected_ = false;
        // 找到的所有的属于这个应用的pid
        std::vector<ProcessInfo> found_process_info_;
        std::shared_ptr<SteamGame> steam_game_ = nullptr;

    };

}


#endif //TC_APPLICATION_APP_MANAGER_WIN_H
