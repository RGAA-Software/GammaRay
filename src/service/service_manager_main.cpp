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
        std::cout << "  -> remove : remove the service" << std::endl;
        std::cout << "  -> query : query the staus of service" << std::endl;
        std::cout << "  -> sr (stop -> remove)" << std::endl;
        std::string command;
        std::cin >> command;

        if (command == "install" || command == "i") {
            g_service_manager->Install();
        }
        else if (command == "remove" || command == "sr") {
            g_service_manager->Remove();
        }
        else if (command == "query" || command == "qr") {
            auto status = g_service_manager->QueryStatus();
            std::cout << "current status: " << (int)status << std::endl;
        }
        else if (command == "exit" || command == "e") {
            exit = true;
        }
    }
    return app.exec();
}