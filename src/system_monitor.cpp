//
// Created by hy on 2024/3/26.
//

#include "system_monitor.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "tc_controller/vigem_driver_manager.h"
#include "tc_controller/vigem/sdk/ViGEm/Client.h"
#include "gr_context.h"

#pragma comment(lib, "version.lib")
#pragma comment(lib, "kernel32.lib")

namespace tc
{

    std::shared_ptr<SystemMonitor> SystemMonitor::Make(const std::shared_ptr<GrContext>& ctx) {
        return std::make_shared<SystemMonitor>(ctx);
    }

    SystemMonitor::SystemMonitor(const std::shared_ptr<GrContext>& ctx) {
        this->ctx_ = ctx;
    }

    void SystemMonitor::Start() {
        vigem_driver_manager_ = VigemDriverManager::Make();

        if (!CheckViGEmDriver()) {
            // install it
        }

        monitor_thread_ = std::make_shared<Thread>([=, this]() {
            while (!exit_) {

                bool vigem_installed = CheckViGEmDriver();
                if (vigem_installed) {
                    if (!TryConnectViGEmDriver()) {
                        // Connect error
                    }
                } else {
                    // not installed
                }

                LOGI("system checking ...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }, "", false);
    }

    void SystemMonitor::Exit() {
        exit_ = true;
        if (monitor_thread_->IsJoinable()) {
            monitor_thread_->Join();
        }
    }

    bool SystemMonitor::CheckViGEmDriver() {
        DWORD major = 0, minor = 0;
        std::wstring path = L"C:\\Windows\\System32\\drivers\\ViGEmBus.sys";

        if (GetFileVersion(path, major, minor)) {
            LOGI("ViGEmBus: {}.{}", major, minor);
            if (major > 1 || (major == 1 && minor >= 17)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool SystemMonitor::GetFileVersion(const std::wstring& filePath, DWORD& major, DWORD& minor){
        DWORD dummy;
        DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &dummy);
        if (size == 0) {
            return false;
        }

        std::vector<BYTE> data(size);
        if (!GetFileVersionInfoW(filePath.c_str(), 0, size, data.data())) {
            return false;
        }

        VS_FIXEDFILEINFO* fileInfo = nullptr;
        UINT fileInfoSize;
        if (!VerQueryValueW(data.data(), L"\\", reinterpret_cast<void**>(&fileInfo), &fileInfoSize)) {
            return false;
        }

        major = (fileInfo->dwFileVersionMS >> 16) & 0xffff;
        minor = (fileInfo->dwFileVersionLS >> 16) & 0xffff;
        return true;
    }

    bool SystemMonitor::TryConnectViGEmDriver() {
        // 1. checking vigem driver is installed or not
        bool detect_ok = false;
        if (!vigem_driver_manager_->IsVigemDriverInstalled()) {
            detect_ok = vigem_driver_manager_->Detect();
        }

        // 2. driver is not exist, to install
        if (vigem_driver_manager_->IsVigemDriverInstalled() && detect_ok) {
            // installed & running
            LOGI("Installed and running");
        } else {
            // 3. driver seems already exists, try to connect
            if (!connect_vigem_success_) {
                connect_vigem_success_ = vigem_driver_manager_->TryConnect();
            }
            // 4. connect failed, to install
            if (!connect_vigem_success_) {
                LOGI("connect failed.");
            } else {
                LOGI("already tested, connect to vigem success.");
                return true;
            }
        }
    }

}