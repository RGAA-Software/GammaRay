#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QLockFile>

#include "tc_common_new/log.h"
#include "tc_common_new/folder_util.h"
#include "gr_application.h"
#include "gr_workspace.h"
#include "util/dxgi_mon_detector.h"

using namespace tc;

bool PrepareDirs(const QString& base_path) {
    std::vector<QString> dirs = {
        "gr_logs", "gr_data"
    };

    bool result = true;
    for (const QString& dir : dirs) {
        auto target_dir_path = base_path + "/" + dir;
        QDir target_dir(target_dir_path);
        if (target_dir.exists()) {
            continue;
        }
        if (!target_dir.mkpath(target_dir_path)) {
            result = false;
            LOGI("Make path failed: {}", target_dir_path.toStdString());
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
//    AllocConsole();
//    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);

    DxgiMonitorDetector::Instance()->DetectAdapters();

    auto base_dir = QString::fromStdWString(FolderUtil::GetCurrentFolderPath());
    auto log_path = base_dir + "/gr_logs/gammaray.log";
    std::cout << "log path: " << log_path.toStdString() << std::endl;
    Logger::InitLog(log_path.toStdString(), true);

    int id = QFontDatabase::addApplicationFont(":/resources/font/matrix.ttf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
    }

    PrepareDirs(base_dir);

    GrWorkspace workspace;
    workspace.setFixedSize(1720, 900);
    workspace.show();

    return app.exec();
}
