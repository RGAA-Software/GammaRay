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
#include <windows.h>
#include "service/service_manager.h"
#include "tc_qt_widget/sized_msg_box.h"

using namespace tc;

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    auto service_manager = ServiceManager::Make();
    service_manager->Init("GammaRayService", "", "GammaRat Service", "** GammaRay Service **");

    QWidget widget;
    widget.setWindowTitle("UNINSTALL");
    widget.setFixedSize(480, 320);
    auto root_layout = new QHBoxLayout();
    auto layout = new QVBoxLayout();
    root_layout->addLayout(layout);
    widget.setLayout(root_layout);
    auto btn_size = QSize(200, 40);
    layout->addStretch();
    // stop all
    {
        auto btn_stop = new QPushButton();
        btn_stop->setFixedSize(btn_size);
        QObject::connect(btn_stop, &QPushButton::clicked, &widget, [=]() {
            auto mbox = SizedMessageBox::MakeOkCancelBox("STOP All Processes", "Do you want to stop all relative processes?");

            if (mbox->exec() == 0) {
                service_manager->Remove();
            }
        });
        btn_stop->setText("STOP ALL");
        layout->addWidget(btn_stop);

    }

    layout->addSpacing(20);

    // uninstall button
    {
        auto btn_uninstall = new QPushButton();
        btn_uninstall->setFixedSize(btn_size);
        QObject::connect(btn_uninstall, &QPushButton::clicked, &widget, [=]() {
            auto mbox = SizedMessageBox::MakeOkCancelBox("** UnInstall **", "Do you want to  UNINSTALL the software?");
            if (mbox->exec() == 0) {
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
            }
        });
        btn_uninstall->setText("UNINSTALL");
        layout->addWidget(btn_uninstall);
    }

    layout->addSpacing(20);

    widget.show();

    return app.exec();
}
