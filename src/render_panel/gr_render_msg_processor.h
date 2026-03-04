//
// Created by RGAA on 26/07/2025.
//

#ifndef GAMMARAYPREMIUM_GR_RENDER_MSG_PROCESSOR_H
#define GAMMARAYPREMIUM_GR_RENDER_MSG_PROCESSOR_H

#include <memory>

namespace tc
{
    class GrContext;
    class Message;

    class GrRenderMsgProcessor {
    public:
        explicit GrRenderMsgProcessor(const std::shared_ptr<GrContext>& ctx);
        void OnMessage(std::shared_ptr<tc::Message> msg) const;

    private:
        std::weak_ptr<GrContext> context_;
    };

}

#endif //GAMMARAYPREMIUM_GR_RENDER_MSG_PROCESSOR_H
