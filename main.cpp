#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QLockFile>

#include "tc_common_new/log.h"
#include "tc_common_new/folder_util.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_workspace.h"
#include "render_panel/gr_running_pipe.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"

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

std::shared_ptr<GrWorkspace> g_workspace = nullptr;

int main(int argc, char *argv[]) {
//    AllocConsole();
//    freopen("CONOUT$", "w", stdout);

    QApplication app(argc, argv);

    auto base_dir = QString::fromStdWString(FolderUtil::GetCurrentFolderPath());
    auto log_path = base_dir + "/gr_logs/gammaray.log";
    std::cout << "log path: " << log_path.toStdString() << std::endl;
    Logger::InitLog(log_path.toStdString(), true);

    // pipe
    auto rn_pipe = std::make_shared<GrRunningPipe>();
    if (!rn_pipe->SendHello()) {
        rn_pipe->StartListening([=]() {
            if (g_workspace) {
                g_workspace->showNormal();
            }
        });
    }

    auto mon_detector = DxgiMonitorDetector::Instance();
    mon_detector->DetectAdapters();
    mon_detector->PrintAdapters();

    int id = QFontDatabase::addApplicationFont(":/src/client/resources/font/SourceHanSansCN-Regular.otf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
        GrSettings::Instance()->def_font_name_ = f.toStdString();
        break;
    }

    PrepareDirs(base_dir);

    g_workspace = std::make_shared<GrWorkspace>();
    g_workspace->setFixedSize(1450, 800);
    g_workspace->show();

    return app.exec();
}
