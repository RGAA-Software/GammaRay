//
// Created by RGAA on 28/07/2025.
//

#include <QWidget>
#include <QApplication>
#include <ShlObj_core.h>
#include "gr_guard_app.h"
#include "gr_guard_context.h"
#include "tc_common_new/log.h"
#include "tc_common_new/auto_start.h"
#include "tc_common_new/shared_preference.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    // Log
    {
        auto path = app.applicationDirPath().toStdString() + "/gr_logs/gammaray_guard.log";
        tc::Logger::InitLog(path, true);
    }

    // SharedPreference
    {
        auto sp_dir = qApp->applicationDirPath() + "/gr_data";
        if (!tc::SharedPreference::Instance()->Init(sp_dir.toStdString(), "gammaray_guard.dat")) {
            //QMessageBox::critical(nullptr, "Error", "You may already run a instance.");
            return -1;
        }
    }

    // AutoStart Task
    {
        auto auto_start = std::make_shared<tc::AutoStart>();
        auto path = QApplication::applicationFilePath().toStdString();
        auto_start->NewLogonTask((char*)"GammaRay_Guard_Start", (char*)path.c_str(), NULL, (char*)"GR");
    }

    auto context = std::make_shared<tc::GrGuardContext>();
    auto guard_app = std::make_shared<tc::GrGuardApp>(context);

    auto w = new QWidget();
    w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_TranslucentBackground, true);
    w->setGeometry(0, 0, 10, 10);
    w->show();

    return app.exec();
}
