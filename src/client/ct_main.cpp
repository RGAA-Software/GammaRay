//
// Created by RGAA on 2023-12-26.
//
#if 0
#include <QDir>
#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>

#include "thunder_sdk.h"
#include "client/ct_client_context.h"
#include "client/ct_workspace.h"
#include "client/ct_application.h"
#include "tc_common_new/log.h"

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

int main(int argc, char** argv) {

#ifdef WIN32
    //CaptureDump();
#endif

#ifdef __APPLE__
    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(3, 3);
    fmt.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(fmt);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif

    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

    // font
    int id = QFontDatabase::addApplicationFont(":/resources/font/SourceHanSansCN-Regular.otf");
    auto families = QFontDatabase::applicationFontFamilies(id);
    for (auto& f : families) {
        LOGI("font family : {}", f.toStdString());
        break;
    }

    PrepareDirs(app.applicationDirPath());

    auto ctx = std::make_shared<ClientContext>("ui");
    ctx->Init(false);
    Application application(ctx);
    application.show();

    return app.exec();
}
#endif