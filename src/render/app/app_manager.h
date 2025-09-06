//
// Created by RGAA on 2023-12-20.
//

#ifndef TC_APPLICATION_APP_MANAGER_H
#define TC_APPLICATION_APP_MANAGER_H

#include <memory>

#include "tc_capture_new/inject_params.h"

namespace tc
{

    class RdContext;
    class MessageListener;
    class SteamManager;

    class AppManager {
    public:

        explicit AppManager(const std::shared_ptr<RdContext>& ctx);
        virtual ~AppManager();

        virtual void Init();
        virtual bool StartProcess();
        virtual bool StartProcessWithHook();
        virtual void Exit();
        virtual void* GetWindowHandle() = 0;
        virtual void CloseCurrentApp();

    protected:
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //TC_APPLICATION_APP_MANAGER_H
