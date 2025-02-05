//
// Created by RGAA on 2023/12/20.
//

#ifndef TC_APPLICATION_HTTP_HANDLER_H
#define TC_APPLICATION_HTTP_HANDLER_H

#include "http/httplib.h"
#include "tc_3rdparty/json/json.hpp"
#include <asio2/asio2.hpp>

using namespace nlohmann;

namespace tc
{

    class GrContext;
    class GrApplication;
    class GrRunGameManager;

    class HttpHandler {
    public:

        explicit HttpHandler(const std::shared_ptr<GrApplication>& app);

        void HandlePing(http::web_request &req, http::web_response &rep);
        void HandleSimpleInfo(http::web_request &req, http::web_response &rep);
        void HandleGames(http::web_request &req, http::web_response &rep);
        void HandleGameStart(http::web_request &req, http::web_response &rep);
        void HandleGameStop(http::web_request &req, http::web_response &rep);
        void HandleRunningGames(http::web_request &req, http::web_response &rep);
        void HandleStopServer(http::web_request &req, http::web_response &rep);
        void HandleAllRunningProcesses(http::web_request &req, http::web_response &rep);
        void HandleKillProcess(http::web_request &req, http::web_response &rep);

    private:
        std::string GetInstalledGamesAsJson();
        std::string WrapBasicInfo(int code, const std::string& msg, const std::string& data);
        std::string WrapBasicInfo(int code, const std::string& msg, const json& data);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrRunGameManager> run_game_mgr_ = nullptr;

    };

}

#endif //TC_APPLICATION_HTTP_HANDLER_H
