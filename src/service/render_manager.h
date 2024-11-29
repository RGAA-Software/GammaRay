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

    class ServiceContext;

    class RunningAppInfo {
    public:
        uint32_t pid_{0};
        std::shared_ptr<QProcess> process_ = nullptr;
    };

    class RenderManager {
    public:

        explicit RenderManager(const std::shared_ptr<ServiceContext>& ctx);
        ~RenderManager();

        Response<bool, uint32_t> StartServer();
        Response<bool, bool> StopServer();
        Response<bool, uint32_t> ReStart();
        void Exit();

    private:
        std::shared_ptr<ServiceContext> context_ = nullptr;
        std::shared_ptr<RunningAppInfo> running_srv_;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
