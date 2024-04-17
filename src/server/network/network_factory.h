//
// Created by hy on 2024/3/1.
//

#ifndef TC_APPLICATION_NETWORK_FACTORY_H
#define TC_APPLICATION_NETWORK_FACTORY_H

#include <memory>
#include "connection.h"
#include "ws_server.h"
#include "app_server.h"

namespace tc
{

    class Application;

    class NetworkFactory {
    public:

        static std::shared_ptr<Connection> MakeConnection(const std::shared_ptr<Application>& app) {
            return std::make_shared<AppServer>(app);
            //return std::make_shared<WSServer>(app);
        }

    };

}

#endif //TC_APPLICATION_NETWORK_FACTORY_H
