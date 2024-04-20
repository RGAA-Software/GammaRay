//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_ROUTER_H
#define TC_APPLICATION_WS_ROUTER_H

#include <map>
#include <string>
#include <functional>
#include <any>
#include <asio2/asio2.hpp>

namespace tc
{

    class Data;

    class WsData {
    public:
        std::map<std::string, std::any> vars_;
    };
    using WsDataPtr = std::shared_ptr<WsData>;

    class WsRouter {
    public:

        explicit WsRouter(const WsDataPtr& ws_data) {
            ws_data_ = ws_data;
        }

        virtual void OnOpen(std::shared_ptr<asio2::http_session>& sess_ptr) {
            session_ = sess_ptr;
            session_->ws_stream().binary(true);
            // how to send a message to client when the websocket connection is connected.
            // can't use session_ptr->async_send(...) directly, because the websocket connection is not ready.
            session_->post_queued_event([=]() {
                //session_->async_send("eg: hello websocket");
            });
        }

        virtual void OnClose(std::shared_ptr<asio2::http_session>& sess_ptr) {
            session_ = nullptr;
        }

        virtual void OnMessage(std::shared_ptr<asio2::http_session>& sess_ptr, std::string_view data) {

        }

        virtual void OnPing(std::shared_ptr<asio2::http_session>& sess_ptr) {

        }

        virtual void OnPong(std::shared_ptr<asio2::http_session>& sess_ptr) {

        }

        virtual void PostBinaryMessage(const std::shared_ptr<Data>& data) = 0;
        virtual void PostBinaryMessage(const std::string& data) = 0;

    protected:

        template<typename T>
        const T Get(const std::string& n) {
            auto v = ws_data_->vars_[n];
            return std::any_cast<T>(v);
        }

    protected:

        std::shared_ptr<WsData> ws_data_ = nullptr;
        std::shared_ptr<asio2::http_session> session_ = nullptr;

    };

    using WsRouterPtr = std::shared_ptr<WsRouter>;

}

#endif //TC_APPLICATION_WS_ROUTER_H
