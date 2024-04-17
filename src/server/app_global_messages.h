//
// Created by hy on 2024/2/27.
//

#ifndef TC_APPLICATION_APP_GLOBAL_MESSAGES_H
#define TC_APPLICATION_APP_GLOBAL_MESSAGES_H

#include <memory>
#include <string>
#include <functional>

namespace tc
{

    enum class AppMessageType {
        kTaskMessage,
        kExitMessage,
    };

    class AppMessage {
    public:
        std::function<void()> task_;
    };

    class AppExitMessage : public AppMessage {
    public:
        int exit_code_{0};
    };

    class AppMessageMaker {
    public:

        static std::shared_ptr<AppMessage> MakeTaskMessage(std::function<void()>&& task) {
            auto task_msg = std::make_shared<AppMessage>();
            task_msg->task_ = task;
            return task_msg;
        }

        static std::shared_ptr<AppExitMessage> MakeExitMessage(int exit_code = 0) {
            auto msg = std::make_shared<AppExitMessage>();
            msg->exit_code_ = exit_code;
            return msg;
        }

    };
}

#endif //TC_APPLICATION_APP_MESSAGES_H
