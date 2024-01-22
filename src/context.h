//
// Created by hy on 2024/1/17.
//

#ifndef TC_SERVER_STEAM_CONTEXT_H
#define TC_SERVER_STEAM_CONTEXT_H

#include <memory>

namespace tc
{

    class SteamManager;
    class TaskRuntime;

    class Context : public std::enable_shared_from_this<Context> {
    public:
        Context();

        void Init();
        std::shared_ptr<SteamManager> GetSteamManager();
        std::shared_ptr<TaskRuntime> GetTaskRuntime();

    private:
        std::shared_ptr<SteamManager> steam_mgr_ = nullptr;
        std::shared_ptr<TaskRuntime> task_runtime_ = nullptr;
    };

}
#endif //TC_SERVER_STEAM_CONTEXT_H
