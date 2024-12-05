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

    class GrContext;
    class GrServiceClient;
    class GrApplication;

    class GrRenderController {
    public:

        explicit GrRenderController(const std::shared_ptr<GrApplication>& app);
        ~GrRenderController();

        bool StartServer();
        bool StopServer();
        bool ReStart();
        void Exit();

    private:
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_MANAGER_H
