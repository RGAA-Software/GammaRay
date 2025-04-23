//
// Created by RGAA on 23/04/2025.
//

#ifndef GAMMARAY_GR_ACCOUNT_MANAGER_H
#define GAMMARAY_GR_ACCOUNT_MANAGER_H

#include <memory>
#include <string>
#include <functional>

#include "gr_account_error.h"
#include "expt/expected.h"

namespace tc
{

    class GrContext;
    class GrSettings;
    class MessageNotifier;
    class MessageListener;

    class GrAccountManager {
    public:
        explicit GrAccountManager(const std::shared_ptr<GrContext>& ctx);

        Result<bool, GrAccountError> IsDeviceInfoOk();

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAY_GR_ACCOUNT_MANAGER_H
