//
// Created by RGAA on 2024/3/1.
//

#ifndef TC_APPLICATION_COMMAND_MANAGER_H
#define TC_APPLICATION_COMMAND_MANAGER_H

#include <memory>
#include <functional>

namespace tc
{

    class Application;

    class CommandManager {
    public:

        static std::shared_ptr<CommandManager> Make(const std::shared_ptr<Application>& app);

        explicit CommandManager(const std::shared_ptr<Application>& app);

    private:
        std::shared_ptr<Application> app_ = nullptr;

    };

}

#endif //TC_APPLICATION_COMMAND_MANAGER_H
