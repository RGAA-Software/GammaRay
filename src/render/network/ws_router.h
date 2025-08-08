//
// Created by RGAA on 2024/3/5.
//

#ifndef TC_APPLICATION_WS_ROUTER_H
#define TC_APPLICATION_WS_ROUTER_H

#include <map>
#include <any>
#include <atomic>
#include <string>
#include <functional>
#include <asio2/asio2.hpp>
#include "ws_data.h"

namespace tc
{
    class Data;

    class WsRouter {
    public:

        explicit WsRouter(const WsDataPtr& ws_data) {
            ws_data_ = ws_data;
        }

        virtual void OnOpen(std::shared_ptr<asio2::http_session>& sess_ptr) {
            session_ = sess_ptr;
        }

        virtual void OnClose(std::shared_ptr<asio2::http_session>& sess_ptr) {
            session_ = nullptr;
        }

        virtual void OnMessage(std::shared_ptr<asio2::http_session>& sess_ptr, int64_t socket_fd, std::string_view data) {

        }

        virtual void OnPing(std::shared_ptr<asio2::http_session>& sess_ptr) {

        }

        virtual void OnPong(std::shared_ptr<asio2::http_session>& sess_ptr) {

        }

        virtual void PostBinaryMessage(const std::shared_ptr<Data>& data) {

        }

        virtual void PostBinaryMessage(const std::string& data) {

        }

        virtual void PostTextMessage(const std::string& data) {

        }

        virtual int64_t GetQueuingMsgCount() {
            return queuing_message_count_;
        }

    protected:

        template<typename T>
        T Get(const std::string& n) {
            auto v = ws_data_->vars_[n];
            return std::any_cast<T>(v);
        }

    protected:
        std::shared_ptr<WsData> ws_data_ = nullptr;
        std::shared_ptr<asio2::http_session> session_ = nullptr;
        std::atomic_int64_t queuing_message_count_ = 0;

    public:
        bool enable_audio_ = false;
        bool enable_video_ = false;

    };

    using WsRouterPtr = std::shared_ptr<WsRouter>;

}

#endif //TC_APPLICATION_WS_ROUTER_H
