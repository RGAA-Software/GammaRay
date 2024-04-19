//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_TC_APP_MANAGER_H
#define TC_SERVER_STEAM_TC_APP_MANAGER_H

#include <memory>
#include <map>
#include <mutex>
#include "tc_common_new/concurrent_hashmap.h"
#include "tc_common_new/response.h"

namespace tc
{

    class GrContext;
    class RunningAppInfo;

    // 用来管理tc_server
    class ServerManager {
    public:

        explicit ServerManager(const std::shared_ptr<GrContext>& ctx);
        ~ServerManager();

        Response<bool, uint32_t> Start();
        Response<bool, bool> Stop(uint32_t pid);
        Response<bool, uint32_t> ReStart();
        void Exit();

    private:

        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<RunningAppInfo> running_srv_;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
