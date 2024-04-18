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
    class AppManager {
    public:

        explicit AppManager(const std::shared_ptr<GrContext>& ctx);

        Response<bool, uint32_t> Start(const std::string& args);
        Response<bool, bool> Stop(uint32_t pid);

    private:

        std::shared_ptr<GrContext> context_ = nullptr;
        tc::ConcurrentHashMap<uint32_t, std::shared_ptr<RunningAppInfo>> running_apps_;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
