//
// Created by RGAA on 2023-12-22.
//

#include "app_manager_factory.h"

#ifdef WIN32
#include "app/win/app_manager_win.h"
#else
#include "app_manager_linux.h"
#endif

namespace tc
{

    std::shared_ptr<AppManager> AppManagerFactory::Make(const std::shared_ptr<Context>& ctx) {
#ifdef WIN32
        auto mgr = std::make_shared<AppManagerWinImpl>(ctx);
        mgr->Init();
        return mgr;
#else
        return nullptr;
#endif
    }

}