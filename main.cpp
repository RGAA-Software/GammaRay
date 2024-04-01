#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "src/model/game_model.h"
#include "application.h"
#include "tc_common_new/log.h"

using namespace tc;

int main(int argc, char *argv[])
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    QGuiApplication app(argc, argv);

    Logger::InitLog("app.log", false);

    auto application = std::make_shared<Application>();
    application->Init();

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("installed_game_model", application->GetInstalledModel());
    engine.loadFromModule("tc_server_steam", "Main");

    return app.exec();
}
