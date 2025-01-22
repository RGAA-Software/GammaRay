//
// Created by RGAA on 22/01/2025.
//

#include <QApplication>
#include <QProcess>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProcess>
#include <QTimer>
#include "service/service_manager.h"
#include <windows.h>

using namespace tc;

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    auto service_manager = ServiceManager::Make();
    service_manager->Init("GammaRayService", "", "GammaRat Service", "** GammaRay Service **");

    QWidget widget;
    widget.setFixedSize(960, 540);
    auto root_layout = new QVBoxLayout();
    widget.setLayout(root_layout);
    // uninstall button
    {
        auto btn_uninstall = new QPushButton();
        QObject::connect(btn_uninstall, &QPushButton::clicked, &widget, [=]() {
            service_manager->Remove();
            QTimer::singleShot(1000, [=]() {
                QString path = QCoreApplication::applicationDirPath();
                path += "/shadow_deleter.exe";
                auto process = new QProcess();
                QStringList args;
                args.append(path);
                process->startDetached(path, args);

                qApp->exit(0);
            });
        });
        btn_uninstall->setText("UNINSTALL");
        root_layout->addWidget(btn_uninstall);
    }
    widget.show();

    return app.exec();
}
