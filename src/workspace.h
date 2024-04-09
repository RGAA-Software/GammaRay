//
// Created by hy on 2024/4/9.
//

#ifndef TC_SERVER_STEAM_WORKSPACE_H
#define TC_SERVER_STEAM_WORKSPACE_H

#include <QMainWindow>
#include <memory>

namespace tc
{

    class Application;

    class Workspace : public QMainWindow {
    public:
        Workspace();

    private:
        std::shared_ptr<Application> app_ = nullptr;
    };

}

#endif //TC_SERVER_STEAM_WORKSPACE_H
