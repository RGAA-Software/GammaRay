//
// Created by RGAA
//

#pragma once
#include <string>

namespace tc
{

    class SignalingParam {
    public:
        std::string host_{};
        int port_{};
        std::string path_{};
    };

}