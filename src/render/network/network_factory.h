//
// Created by RGAA on 2024/3/1.
//

#ifndef TC_APPLICATION_NETWORK_FACTORY_H
#define TC_APPLICATION_NETWORK_FACTORY_H

// DISABLED
#if 0

#include <memory>
#include "connection.h"
#include "app_server.h"

namespace tc
{

    class RdApplication;

    class NetworkFactory {
    public:

        static std::shared_ptr<Connection> MakeConnection(const std::shared_ptr<RdApplication>& app) {
            return std::make_shared<AppServer>(app);
        }

    };

}
#endif

#endif //TC_APPLICATION_NETWORK_FACTORY_H
