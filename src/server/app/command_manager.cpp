//
// Created by hy on 2024/3/1.
//

#include "command_manager.h"
#include "app.h"

namespace tc
{
    std::shared_ptr<CommandManager> CommandManager::Make(const std::shared_ptr<Application>& app) {
        return std::make_shared<CommandManager>(app);
    }

    CommandManager::CommandManager(const std::shared_ptr<Application>& app) {
        this->app_ = app;
    }
}