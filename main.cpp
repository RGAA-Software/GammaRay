#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>

#include "tc_common_new/log.h"
#include "tc_common_new/folder_util.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_workspace.h"
#include "render_panel/gr_running_pipe.h"
#include "render_panel/gr_settings.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_qt_widget/translator/tc_translator.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "tc_common_new/shared_preference.h"

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
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication app(argc, argv);

    //auto base_dir = QString::fromStdWString(FolderUtil::GetCurrentFolderPath());
    auto base_dir = QApplication::applicationDirPath();
    PrepareDirs(base_dir);

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

    tcFontMgr()->InitFont(":/src/client/resources/font/ms_yahei.ttf");

    // init sp
    auto sp_dir = qApp->applicationDirPath() + "/gr_data";
    if (!SharedPreference::Instance()->Init(sp_dir.toStdString(), "gammaray.dat")) {
        QMessageBox::critical(nullptr, "Error", "Init failed");
        return -1;
    }

    // init language
    tcTrMgr()->InitLanguage();

    g_workspace = std::make_shared<GrWorkspace>();
    g_workspace->setFixedSize(1450, 800);
    g_workspace->show();

    return app.exec();
}
