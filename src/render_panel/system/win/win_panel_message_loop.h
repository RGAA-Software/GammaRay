#pragma once
#include <Windows.h>
#include <memory>
#include <thread>

namespace tc
{

    class GrContext;
    class WinMessageWindow;

    class WinMessageLoop : public std::enable_shared_from_this<WinMessageLoop> {
    public:
        static std::shared_ptr<WinMessageLoop> Make(const std::shared_ptr<GrContext>& ctx);
        explicit WinMessageLoop(const std::shared_ptr<GrContext>& ctx);
        ~WinMessageLoop();
        void Start();
        void Stop();

        void OnClipboardUpdate(HWND hwnd);
        void OnDisplayDeviceChange();
    private:
        void CreateMessageWindow();
        void ThreadFunc();
        void OnWinSessionChange(uint32_t msg);
        static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::thread thread_;
        std::shared_ptr<WinMessageWindow> message_window_ = nullptr;
    };

}