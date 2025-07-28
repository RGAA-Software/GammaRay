//
// Created by RGAA on 28/07/2025.
//

#ifndef GAMMARAYPREMIUM_GR_GUARD_STARTER_H
#define GAMMARAYPREMIUM_GR_GUARD_STARTER_H

#include <memory>

namespace tc
{

    class GrContext;
    class MessageListener;

    class GrGuardStarter {
    public:
        explicit GrGuardStarter(const std::shared_ptr<GrContext>& ctx);

    private:
        bool CheckGuardState();
        void StartGuard();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_GUARD_STARTER_H
