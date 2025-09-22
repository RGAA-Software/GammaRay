//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_TC_APP_MANAGER_H
#define TC_SERVER_STEAM_TC_APP_MANAGER_H

#include <memory>
#include <map>
#include <mutex>
#include <QProcess>
#include "render_process.h"
#include "tc_common_new/response.h"
#include "tc_common_new/concurrent_hashmap.h"

namespace tc
{

    static const std::string kGammaRayName = "GammaRay.exe";
    static const std::string kGammaRayGuardName = "GammaRayGuard.exe";
    static const std::string kGammaRayRenderName = "GammaRayRender.exe";
    static const std::string kGammaRayClientInner = "GammaRayClientInner.exe";
    static const std::string kGammaRaySysInfo = "gr_sysinfo.exe";

    class ServiceContext;
    class MessageListener;
    class ProcessInfo;

    class RenderManager {
    public:
        explicit RenderManager(const std::shared_ptr<ServiceContext>& ctx);
        ~RenderManager();

        // desktop
        bool StartDesktopRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        bool StopDesktopRender();
        bool ReStartDesktopRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        [[nodiscard]] bool IsDesktopRenderAlive() const;

        // others
        bool StartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        bool ReStartRender(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        bool StopRender(RenderProcessId id);

        void Exit();

    private:
        void CheckAliveRenders(const std::vector<std::shared_ptr<ProcessInfo>>& processes);
        // desktop
        bool StartDesktopRenderInternal(const std::string& work_dir, const std::string& app_path, const std::string& args);
        bool CheckPanelAlive(const std::vector<std::shared_ptr<ProcessInfo>>& processes);

        // others

    private:
        std::shared_ptr<ServiceContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        // desktop
        bool is_desktop_render_alive_ = false;
        std::shared_ptr<ProcessInfo> desktop_render_process_ = nullptr;
        std::string desktop_app_path_;
        std::string desktop_app_args_;
        std::string desktop_work_dir_;

        // others
        tc::ConcurrentHashMap<RenderProcessId, std::shared_ptr<RenderProcess>> render_processes_;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
