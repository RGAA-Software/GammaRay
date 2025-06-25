//
// Created by RGAA on 19/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_CONTEXT_H
#define GAMMARAY_GR_PLUGIN_CONTEXT_H

#include <QObject>
#include <functional>
#include <memory>
#include <string>

namespace asio2
{
    class timer;
}

namespace tc
{

    class Thread;

    class ClientPluginContext : public QObject {
    public:
        explicit ClientPluginContext(const std::string& plugin_name);
        ~ClientPluginContext() override = default;

        void OnDestroy();

        // tasks
        void PostWorkTask(std::function<void()>&& task);
        void PostUITask(std::function<void()>&& task);

        // timer
        void StartTimer(int millis, std::function<void()>&& cbk);

    private:
        std::shared_ptr<Thread> work_thread_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
    };

}

#endif //GAMMARAY_GR_PLUGIN_CONTEXT_H
