#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "src/model/game_model.h"
#include "application.h"

using namespace tc;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    Application application;
    application.Init();

    QQmlApplicationEngine engine;
    QObject::connect(
            &engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection);

    auto game_model = new tc::GameModel();
    for (int i = 0; i < 30; i++) {
        game_model->AddGame("Good...", 10);
    }

    engine.rootContext()->setContextProperty("game_model", game_model);
    engine.loadFromModule("tc_server_steam", "Main");

    return app.exec();
}
