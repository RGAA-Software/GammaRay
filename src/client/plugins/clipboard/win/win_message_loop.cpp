#include "win_message_loop.h"
#include <iostream>
#include <wtsapi32.h>
#include "tc_common_new/log.h"
#include "win_message_window.h"
#include "rd_context.h"
#include "plugins/plugin_manager.h"
#include "render/app/app_messages.h"
#include "plugin_interface/gr_monitor_capture_plugin.h"

namespace tc
{

    constexpr char kWindowName[] = "GammaRay_client_MessageWindow";

    void CALLBACK WinMessageLoop::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject,
                                               LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
        if (event == EVENT_SYSTEM_DESKTOPSWITCH) {
            std::cout << "Desktop switch event detected." << std::endl;
        }
    }

    std::shared_ptr<WinMessageLoop> WinMessageLoop::Make(ClientClipboardPlugin* plugin) {
        return std::make_shared<WinMessageLoop>(plugin);
    }

    WinMessageLoop::WinMessageLoop(ClientClipboardPlugin* plugin) {
        plugin_ = plugin;
    }

    WinMessageLoop::~WinMessageLoop() {

    }

    void WinMessageLoop::CreateMessageWindow() {
        //构造函数内 不能使用shared_from_this();
        message_window_ = WinMessageWindow::Make(plugin_, shared_from_this());
    }

    void WinMessageLoop::OnWinSessionChange(uint32_t message) {

    }

    void WinMessageLoop::OnDisplayDeviceChange() {

    }

    void WinMessageLoop::Start() {
        CreateMessageWindow();
        thread_ = std::thread(std::bind(&WinMessageLoop::ThreadFunc, this));
    }

    void WinMessageLoop::Stop() {
        LOGI("WinMessageLoop stopping...");
        message_window_->CloseWindow();
        if (thread_.joinable()) {
            thread_.join();
        }
        LOGI("WinMessageLoop stoped.");
    }

    void WinMessageLoop::ThreadFunc() {
        if (!message_window_->Create(kWindowName)) {
            LOGE("WinMessageLoop create window error.");
            return;
        }
        LOGI("WinMessageWindow create success");
        HWND hwnd = nullptr;
        hwnd = message_window_->GetHwnd();
        if (!hwnd) {
            LOGE("WinMessageLoop hwnd is nullptr.");
            return;
        }

        AddClipboardFormatListener(hwnd);
        LOGI("AddClipboardFormatListener already add WinMessageWindow");

        if (!WTSRegisterSessionNotification(hwnd, NOTIFY_FOR_ALL_SESSIONS)) {
            LOGE("WTSRegisterSessionNotification error: %d", GetLastError());
            return;
        }

        HWINEVENTHOOK hEventHook = SetWinEventHook(EVENT_SYSTEM_DESKTOPSWITCH, EVENT_SYSTEM_DESKTOPSWITCH, nullptr, &WinMessageLoop::WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

        if (hEventHook == nullptr) {
            std::cout << "Failed to set event hook." << std::endl;
            return;
        }

        int bRet = 0;
        MSG msg{};
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
            if (bRet == -1) {
                break;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        UnhookWinEvent(hEventHook);
    }

}