//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_TC_APP_MANAGER_H
#define TC_SERVER_STEAM_TC_APP_MANAGER_H

#include <memory>
#include <map>
#include <mutex>

#include <QProcess>

#include "tc_common_new/concurrent_hashmap.h"
#include "tc_common_new/response.h"

namespace tc
{

    static const std::string kGammaRayName = "GammaRay.exe";
    static const std::string kGammaRayRenderName = "GammaRayRender.exe";
    static const std::string kGammaRayClient = "GammaRayClient.exe";
    static const std::string kGammaRayClientInner = "GammaRayClientInner.exe";

    class ServiceContext;
    class MessageListener;
    class ProcessInfo;

    class RenderManager {
    public:

        explicit RenderManager(const std::shared_ptr<ServiceContext>& ctx);
        ~RenderManager();

        bool StartServer(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        bool StopServer();
        bool ReStart(const std::string& work_dir, const std::string& app_path, const std::vector<std::string>& args);
        void Exit();
        [[nodiscard]] bool IsRenderAlive() const;

    private:
        bool CheckRenderAlive();
        bool StartServerInternal(const std::string& work_dir, const std::string& app_path, const std::string& args);

    private:
        std::shared_ptr<ServiceContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        bool is_render_alive_ = false;
        std::shared_ptr<ProcessInfo> render_process_ = nullptr;
        std::string app_path_;
        std::string app_args_;
        std::string work_dir_;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
