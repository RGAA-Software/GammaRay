#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>

#include "tc_common_new/log.h"
#include "gr_application.h"
#include "gr_workspace.h"
#include "util/dxgi_mon_detector.h"

using namespace tc;

int main(int argc, char *argv[]) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);
    Logger::InitLog("GammaRay.log", false);

    int id = QFontDatabase::addApplicationFont(":/resources/font/matrix.ttf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
    }

    DxgiMonitorDetector::Instance()->DetectAdapters();
    GrWorkspace workspace;
    workspace.setFixedSize(1820, 960);
    workspace.show();

    return app.exec();
}
