#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>

#include "tc_common_new/log.h"
#include "gr_application.h"
#include "workspace.h"
#include "util/dxgi_mon_detector.h"

using namespace tc;

void LoadStyle(const std::string &name) {
    auto qssFile = ":/qss/lightblue.css";

    QString qss;
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly)) {
        qDebug() << "open success...";
        QStringList list;
        QTextStream in(&file);

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
}

int main(int argc, char *argv[])
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);

    Logger::InitLog("GammaRay.log", false);

    int id = QFontDatabase::addApplicationFont(":/resources/font/matrix.ttf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
    }

    LoadStyle("");
    DxgiMonitorDetector::Instance()->DetectAdapters();
    Workspace workspace;
    workspace.setFixedSize(1820, 960);
    workspace.show();

    return app.exec();
}
