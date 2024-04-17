//
// Created by RGAA on 2023-12-20.
//

#ifndef TC_APPLICATION_APP_MANAGER_H
#define TC_APPLICATION_APP_MANAGER_H

#include <memory>

#include "tc_capture_new/win/inject_params.h"

namespace tc
{

    class Context;
    class MessageListener;
    class SteamManager;

    class AppManager {
    public:

        explicit AppManager(const std::shared_ptr<Context>& ctx);
        virtual ~AppManager();

        virtual void Init();
        virtual bool StartProcess();
        virtual bool StartProcessWithHook();
        virtual void Exit();
        virtual void* GetWindowHandle() = 0;
        virtual void CloseCurrentApp();

    private:
        //void InitSteamManager();

    protected:

        std::shared_ptr<Context> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        //std::shared_ptr<SteamManager> steam_manager_ = nullptr;

    };

}

#endif //TC_APPLICATION_APP_MANAGER_H
