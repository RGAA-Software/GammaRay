//
// Created by hy on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include "http/httplib.h"
#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    class GrContext;
    class GrApplication;

    class HttpHandler {
    public:

        explicit HttpHandler(const std::shared_ptr<GrApplication>& app);

        void HandlePing(const httplib::Request& req, httplib::Response& res);
        void HandleSimpleInfo(const httplib::Request& req, httplib::Response& res);
        void HandleSupportApis(const httplib::Request& req, httplib::Response& res);
        void HandleGames(const httplib::Request& req, httplib::Response& res);
        void HandleGameStart(const httplib::Request& req, httplib::Response& res);
        void HandleGameStop(const httplib::Request& req, httplib::Response& res);
        void HandleGameStartOnly(const httplib::Request& req, httplib::Response& res);
        void HandleGameStopOnly(const httplib::Request& req, httplib::Response& res);

    private:
        std::string GetInstalledGamesAsJson();
        std::string WrapBasicInfo(int code, const std::string& msg, const std::string& data);
        std::string WrapBasicInfo(int code, const std::string& msg, const json& data);

    private:

        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H
