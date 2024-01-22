//
// Created by hy on 2023/12/25.
//

#ifndef TC_APPLICATION_SESSION_H
#define TC_APPLICATION_SESSION_H

#include <cstdint>
#include <memory>
#include <functional>
#include <uwebsockets/App.h>

#include "personal_data.h"

namespace tc
{

    using WebsocketConn = uWS::WebSocket<false, true, PerSocketData>;

    class Session {
    public:

        static std::shared_ptr<Session> Make(WebsocketConn* conn);

        explicit Session(WebsocketConn* conn);
        ~Session();

        WebsocketConn* GetConnection();
        void SendNetMessage(const std::string& msg);

    private:
        uint64_t native_handle_;
        WebsocketConn* conn_ = nullptr;
    };

}

#endif //TC_APPLICATION_SESSION_H
