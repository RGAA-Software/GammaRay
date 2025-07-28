//
// Created by RGAA on 28/07/2025.
//

#include <QWidget>
#include <QApplication>
#include "tc_common_new/auto_start.h"
#include "gr_guard_app.h"
#include "gr_guard_context.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    {
        auto auto_start = std::make_shared<tc::AutoStart>();
        auto path = QApplication::applicationFilePath().toStdString();
        auto_start->NewTask((char*)"GammaRay_Guard_Start", (char*)path.c_str(), NULL, (char*)"GR");
    }

    auto context = std::make_shared<tc::GrGuardContext>();
    auto guard_app = std::make_shared<tc::GrGuardApp>(context);

    auto w = new QWidget();
    w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    w->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_TranslucentBackground, true);
    w->setFixedSize(11, 11);
    w->show();

    return app.exec();
}
