#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>
//#include <QQmlApplicationEngine>
//#include <QQmlContext>

#include "src/model/game_model.h"
#include "tc_common_new/log.h"
#include "application.h"
#include "workspace.h"

using namespace tc;

void LoadStyle(const std::string &name) {
    QElapsedTimer time;
    time.start();

    auto qssFile = ":/qss/lightblue.css";

    QString qss;
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly)) {
        qDebug() << "open success...";
        QStringList list;
        QTextStream in(&file);
        //in.setCodec("utf-8");
        while (!in.atEnd()) {
            QString line;
            in >> line;
            list << line;
        }

        file.close();
        qss = list.join("\n");
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(paletteColor));
        qApp->setStyleSheet(qss);
    }

    qDebug() << "用时:" << time.elapsed();
}

int main(int argc, char *argv[])
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);

    Logger::InitLog("app.log", false);

    int id = QFontDatabase::addApplicationFont(":/resources/font/matrix.ttf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
    }

    LoadStyle("");

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
