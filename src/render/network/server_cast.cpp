//
// Created by RGAA on 2024/3/15.
//

#include "server_cast.h"
#include "tc_common_new/log.h"
#include "context.h"

namespace tc
{

    std::shared_ptr<ServerCast> ServerCast::Make(std::shared_ptr<Context> &ctx) {
        return std::make_shared<ServerCast>(ctx);
    }

    ServerCast::ServerCast(const std::shared_ptr<Context> &ctx) : context_(ctx) {

    }

    void ServerCast::Start() {

    }

    void ServerCast::Stop() {

    }

}
