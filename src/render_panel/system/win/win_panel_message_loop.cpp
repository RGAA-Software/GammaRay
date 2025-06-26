#include "win_panel_message_loop.h"
#include <iostream>
#include <wtsapi32.h>
#include <QUrl>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include "tc_common_new/log.h"
#include "render_panel/gr_context.h"
#include "win_panel_message_window.h"

namespace tc
{

    constexpr char kWindowName[] = "GammaRay_panel_MessageWindow";


    void CALLBACK WinMessageLoop::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
    {
        if (event == EVENT_SYSTEM_DESKTOPSWITCH)
        {
            std::cout << "Desktop switch event detected." << std::endl;
        }
    }

    std::shared_ptr<WinMessageLoop> WinMessageLoop::Make(const std::shared_ptr<GrContext>& ctx) {
        return std::make_shared<WinMessageLoop>(ctx);
    }

    WinMessageLoop::WinMessageLoop(const std::shared_ptr<GrContext>& ctx) {
        context_ = ctx;
    }

    WinMessageLoop::~WinMessageLoop() {

    }

    void WinMessageLoop::CreateMessageWindow() {
        message_window_ = WinMessageWindow::Make(context_, shared_from_this());
    }

    void WinMessageLoop::OnClipboardUpdate(HWND hwnd) {
        LOGI("--OnClipboardUpdated.");
        QClipboard *board = QGuiApplication::clipboard();
        auto mime_data = const_cast<QMimeData*>(board->mimeData());
        bool has_urls = mime_data->hasUrls();
        LOGI("has urls: {}", has_urls);

        auto urls = mime_data->urls();
        for (const auto& url : urls) {
            LOGI("url: {}", url.toLocalFile().toStdString());
        }

//        if (auto plugin = plugin_mgr_->GetClipboardPlugin(); plugin) {
//            auto event = std::make_shared<MsgClipboardUpdate>();
//            event->hwnd_ = hwnd;
//            plugin->DispatchAppEvent(event);
//        }
    }

    void WinMessageLoop::OnWinSessionChange(uint32_t message) {

    }

    void WinMessageLoop::OnDisplayDeviceChange() {
//        if (auto plugin = plugin_mgr_->GetDDACapturePlugin(); plugin) {
//            if (plugin->IsPluginEnabled()) {
//                auto event = std::make_shared<MsgDisplayDeviceChange>();
//                LOGI("OnDisplayDeviceChange, DispatchAppEvent is MsgDisplayDeviceChange");
//                plugin->DispatchAppEvent(event);
//                return;
//            }
//        }
//        if (auto plugin = plugin_mgr_->GetGdiCapturePlugin(); plugin) {
//            if (plugin->IsPluginEnabled()) {
//                auto event = std::make_shared<MsgDisplayDeviceChange>();
//                LOGI("OnDisplayDeviceChange, DispatchAppEvent is MsgDisplayDeviceChange");
//                plugin->DispatchAppEvent(event);
//                return;
//            }
//        }
//
//        {
//            MsgReCreateRefresher msg;
//            context_->SendAppMessage(msg);
//        }
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
        // 为了获取windows消息，创建创建一个窗口，参考 https://github.com/dchapyshev/aspia
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

        if (hEventHook == nullptr)
        {
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