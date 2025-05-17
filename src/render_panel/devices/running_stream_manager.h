//
// Created by RGAA on 28/03/2025.
//

#ifndef GAMMARAY_RUNNING_STREAM_MANAGER_H
#define GAMMARAY_RUNNING_STREAM_MANAGER_H

#include <memory>
#include <string>
#include <map>

#include <QProcess>
#include "stream_item.h"

namespace tc
{

    class GrContext;
    class GrSettings;

    class RunningStreamManager {
    public:
        explicit RunningStreamManager(const std::shared_ptr<GrContext>& ctx);
        void StartStream(const std::shared_ptr<StreamItem>& item);
        void StopStream(const std::shared_ptr<StreamItem>& item);

    private:
        GrSettings* settings_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::map<std::string, std::shared_ptr<QProcess>> running_processes_;
    };

}

#endif //GAMMARAY_RUNNING_STREAM_MANAGER_H
