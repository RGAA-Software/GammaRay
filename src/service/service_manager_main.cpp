//
// Created by RGAA on 22/10/2024.
//

#include <format>
#include <iostream>
#include "service/service_manager.h"
#include <Windows.h>
#include <QApplication>
#include <QProcess>

using namespace tc;
std::shared_ptr<tc::ServiceManager> g_service_manager;

// 1. install

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    std::string base_path = app.applicationDirPath().toStdString();
    std::cout << "path: " << base_path << std::endl;
    std::string bin_path = std::format("{}/GammaRayService.exe", base_path);
    std::cout << "binpath: " << bin_path << std::endl;

    g_service_manager = ServiceManager::Make();
    g_service_manager->Init("GammaRayService", bin_path, "GammaRat Service", "** GammaRay Service **");

    bool exit = false;
    while (!exit) {
        std::cout << "please input command:" << std::endl;
        std::cout << "supported commands are: " << std::endl;
        std::cout << "  -> install : install the service" << std::endl;
        std::cout << "  -> start : start the service" << std::endl;
        std::cout << "  -> stop : stop the service " << std::endl;
        std::cout << "  -> remove : remove the service" << std::endl;
        std::cout << "  -> query : query the staus of service" << std::endl;
        std::cout << "  -> sr (stop -> remove)" << std::endl;
        std::cout << "  -> test-exe : start a exe for test" << std::endl;
        std::string command;
        std::cin >> command;

        if (command == "install" || command == "i") {
            g_service_manager->Install();

//            QProcess process;
//
//            QStringList arguments;
//            arguments << "create" << "GammaRayService" << "binPath=" << "D:/source/GammaRay/cmake-build-relwithdebinfo/GammaRayService.exe" << "start=auto" << "DisplayName=GammaRat Service";
//
//            process.start("sc", arguments);
//            process.waitForFinished();
//
//            QByteArray result = process.readAllStandardOutput();
//            QString resultString = QString::fromUtf8(result);
//
//            qDebug() << "Command output:" << resultString;

        }
        else if (command == "start") {
            g_service_manager->Start();
        }
        else if (command == "stop") {
            g_service_manager->Stop();
        }
        else if (command == "remove") {
            g_service_manager->Remove();
        }
        else if (command == "query" || command == "qr") {
            auto status = g_service_manager->QueryStatus();
            std::cout << "current status: " << (int)status << std::endl;
        }
        else if (command == "sr") {
            g_service_manager->RemoveImmediately();
        }
        else if (command == "exit" || command == "e") {
            exit = true;
        }
    }
    return app.exec();
}