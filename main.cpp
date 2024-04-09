#include <QApplication>
//#include <QQmlApplicationEngine>
//#include <QQmlContext>

#include "src/model/game_model.h"
#include "tc_common_new/log.h"
#include "application.h"
#include "workspace.h"

using namespace tc;

int main(int argc, char *argv[])
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);

    Logger::InitLog("app.log", false);

    Workspace workspace;
    workspace.resize(1450, 800);
    workspace.show();

//    QQmlApplicationEngine engine;
//    QObject::connect(
//            &engine,
//            &QQmlApplicationEngine::objectCreationFailed,
//            &app,
//            []() { QCoreApplication::exit(-1); },
//            Qt::QueuedConnection);
//
//    engine.rootContext()->setContextProperty("installed_game_model", application->GetInstalledModel());
//    engine.loadFromModule("tc_server_steam", "Main");

    return app.exec();
}
