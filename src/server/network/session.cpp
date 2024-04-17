//
// Created by hy on 2023/12/25.
//

#include "session.h"

namespace tc
{

    std::shared_ptr<Session> Session::Make(WebsocketConn* conn) {
        return std::make_shared<Session>(conn);
    }

    Session::Session(WebsocketConn* conn) {
        this->conn_ = conn;
        this->native_handle_ = reinterpret_cast<uint64_t>(conn->getNativeHandle());
    }

    Session::~Session() {

    }

    WebsocketConn* Session::GetConnection() {
        return conn_;
    }

    void Session::SendNetMessage(const std::string& msg) {
        if (conn_) {
            conn_->send(msg);
        }
    }

}
