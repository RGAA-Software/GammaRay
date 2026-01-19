#include <QApplication>
#include <QFontDatabase>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>
#include "gflags/gflags.h"
#include "version_config.h"
#include "tc_common_new/log.h"
#include "tc_common_new/auto_start.h"
#include "tc_common_new/folder_util.h"
#include "render_panel/gr_application.h"
#include "render_panel/gr_workspace.h"
#include "render_panel/gr_running_pipe.h"
#include "render_panel/gr_settings.h"
#include "render_panel/util/opengl_helper.h"
#include "tc_common_new/win32/dxgi_mon_detector.h"
#include "tc_qt_widget/translator/tc_translator.h"
#include "tc_qt_widget/tc_font_manager.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/dump_helper.h"
#include "tc_common_new/hardware.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/file_util.h"
#include "tc_common_new/folder_util.h"

using namespace tc;

DEFINE_bool(run_automatically, false, "run when logon");

std::shared_ptr<GrWorkspace> g_workspace = nullptr;

bool PrepareDirs(const QString& base_path) {
    std::vector<QString> dirs = {
        "gr_logs", "gr_data", "gr_data/client", "gr_data/render", "gr_data/panel",
        "gr_data/cache", "gr_dumps"
    };

    bool result = true;
    for (const QString& dir : dirs) {
        auto target_dir_path = base_path + "/" + dir;
        FolderUtil::CreateDir(target_dir_path.toStdString());
    }
    return result;
}

int main(int argc, char *argv[]) {

    tc::Hardware::AcquirePermissionForRestartDevice();

    // run in high level
    tc::ProcessUtil::SetProcessInHighLevel();

    //::ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
    //::ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
    //::ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);  // WM_COPYGLOBALDATA

    QApplication app(argc, argv);

    //CaptureDump();
    // Breakpad
    auto bc = BreakpadContext {
        .version_ = PROJECT_VERSION,
        .app_name_ = qApp->applicationName().toStdString(),
    };
    ClearOldDumps();
    CaptureDumpByBreakpad(&bc);

    //auto base_dir = QApplication::applicationDirPath();
    auto base_dir = QString::fromStdWString(FolderUtil::GetProgramDataPath());
    PrepareDirs(base_dir);

    auto log_path = base_dir + "/gr_logs/gammaray.log";
    Logger::InitLog(log_path.toStdString(), true);

    // Check OpenGL Backend
    EOpenGLBackend backend = DetectOpenGLBackend();
    LOGI("Detected backend:", (int)backend);
    QString gl_backend = "angle";
    // 设置 QT_OPENGL 环境变量
    switch (backend) {
        case EOpenGLBackend::kDesktop: {
            QString backend = "desktop";
            gl_backend = backend;
            break;
        }
        case EOpenGLBackend::kGLES: {
            QString backend = "angle";
            gl_backend = backend;
            break;
        }
        case EOpenGLBackend::kSoftware: {
            QString backend = "software";
            gl_backend = backend;
            break;
        }
        default: {
            QString backend = "angle";
            gl_backend = backend;
            break;
        }
    }
    GrSettings::Instance()->gl_backend_ = gl_backend.toStdString();
    LOGI("gl_backed:{}", gl_backend.toStdString());

    // params
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // run when logon?
    auto run_automatically = FLAGS_run_automatically;
    LOGI("Commands:");
    LOGI("  Run automatically: {}", run_automatically);

    // pipe
    auto rn_pipe = std::make_shared<GrRunningPipe>();
    if (!rn_pipe->SendHello()) {
        rn_pipe->StartListening([=]() {
            if (g_workspace) {
                g_workspace->showNormal();
                g_workspace->raise();
            }
        });
    }

    // init sp
    auto data_dir = base_dir + "/gr_data";
    if (!SharedPreference::Instance()->Init(data_dir.toStdString(), "gammaray.dat")) {
        // QMessageBox::critical(nullptr, "Error", "You may already run a instance.");
        return -1;
    }

    GrSettings::Instance()->gr_data_path_ = data_dir.toStdString();

    {
        auto auto_start = std::make_shared<tc::AutoStart>();
        auto path = QApplication::applicationFilePath().toStdString();
        auto_start->NewLogonTask((char*)"GammaRay_Panel_Start", (char*)path.c_str(), (char*)"--run_automatically=true", (char*)"GR");

        //auto guard_path = QApplication::applicationDirPath() + "/" + kGammaRayGuardName.c_str();
        //auto_start->NewTimeTask((char*)"GammaRay_Guard_Time_02", (char*)guard_path.toStdString().c_str(), NULL, (char*)"GR");
    }

    auto mon_detector = DxgiMonitorDetector::Instance();
    mon_detector->DetectAdapters();
    mon_detector->PrintAdapters();

    tcFontMgr()->InitFont(":/src/client/resources/font/ms_yahei.ttf");

    // init language
    tcTrMgr()->InitLanguage();

    g_workspace = std::make_shared<GrWorkspace>(run_automatically);
    g_workspace->Init();
    g_workspace->setFixedSize(1450, 800);
    if (!run_automatically) {
        g_workspace->show();
    }

    return app.exec();
}
