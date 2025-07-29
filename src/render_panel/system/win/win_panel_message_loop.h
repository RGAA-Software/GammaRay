#pragma once
#include <Windows.h>
#include <memory>
#include <thread>
#include <QString>

namespace tc
{

    class GrContext;
    class GrApplication;
    class WinMessageWindow;
    class MessageListener;

    class WinMessageLoop : public std::enable_shared_from_this<WinMessageLoop> {
    public:
        explicit WinMessageLoop(const std::shared_ptr<GrApplication>& ctx);
        ~WinMessageLoop();
        void Start();
        void Stop();

        void OnClipboardUpdate(HWND hwnd);
    private:
        void CreateMessageWindow();
        void ThreadFunc();
        void OnWinSessionChange(uint32_t msg);
        static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
    private:
        std::thread thread_;
        QString remote_info_;
        std::shared_ptr<GrApplication> app_ = nullptr;
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<WinMessageWindow> message_window_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}