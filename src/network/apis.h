//
// Created by hy on 2023/12/20.
//

#ifndef TC_APPLICATION_APIS_H
#define TC_APPLICATION_APIS_H

#include "tc_3rdparty/json/json.hpp"

using namespace nlohmann;

namespace tc
{

    static std::string GetSupportedApis() {
        std::stringstream ss;
        ss << " * 表示参数必须要填写" << std::endl;
        {
            ss << "===>" << std::endl;
            ss << "path: /v1/supported/apis" << std::endl;
            ss << "method: GET" << std::endl;
            ss << "desc: 当前支持的所有API" << std::endl;
            ss << "params: None" << std::endl;
        }
        {
            ss << "===>" << std::endl;
            ss << "path: /v1/supported/apis" << std::endl;
            ss << "method: POST" << std::endl;
            ss << "desc: 当前支持的所有API" << std::endl;
            ss << "params: " << std::endl;
            ss << "  * [version]=[string], desc: [版本号], eg: version=1.1" << std::endl;
        }
        return ss.str();
    }

}

#endif //TC_APPLICATION_APIS_H
