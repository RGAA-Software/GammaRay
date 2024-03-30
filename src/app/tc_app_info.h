//
// Created by RGAA on 2024-03-30.
//

#ifndef TC_SERVER_STEAM_TC_APP_INFO_H
#define TC_SERVER_STEAM_TC_APP_INFO_H

#include <cstdint>
#include <memory>
#include <QProcess>

namespace tc
{

    class RunningAppInfo {
    public:
        uint32_t pid_{0};
        std::shared_ptr<QProcess> process_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_TC_APP_INFO_H
