//
// Created by RGAA on 2023/12/20.
//

#ifndef TC_APPLICATION_APIS_H
#define TC_APPLICATION_APIS_H

#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    constexpr auto kNetOk = 200;
    constexpr auto kStartFailed = 600;

    const std::string kPathPing = "/v1/ping";
    const std::string kPathSimpleInfo = "/v1/simple/info";
    const std::string kPathSupportApis = "/v1/support/apis";
    const std::string kPathGames = "/v1/games";
    const std::string kPathGameStart = "/v1/game/start";
    const std::string kPathGameStop = "/v1/game/stop";
    const std::string kPathRunningGames = "/v1/running/games";

    static std::string GetSupportedApis() {
        std::stringstream ss;
        ss << "* => necessary param" << std::endl;
        {
            ss << "===>" << std::endl;
            ss << "path:" << kPathSupportApis << std::endl;
            ss << "method: GET" << std::endl;
            ss << "desc: to get all apis' information" << std::endl;
            ss << "params: None" << std::endl;
        }
        {
            ss << "===>" << std::endl;
            ss << "path:" << kPathPing << std::endl;
            ss << "method: GET" << std::endl;
            ss << "desc: response a 'Pong'" << std::endl;
            ss << "params: None" << std::endl;
        }
        return ss.str();
    }

}

#endif //TC_APPLICATION_APIS_H
