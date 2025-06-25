#pragma once
#include <Windows.h>
#include <memory>
#include <thread>

namespace tc
{

    class WinMessageWindow;
    class ClientClipboardPlugin;

    // 监听windows消息
    class WinMessageLoop : public std::enable_shared_from_this<WinMessageLoop> {
    public:
        static std::shared_ptr<WinMessageLoop> Make(ClientClipboardPlugin* plugin);
        explicit WinMessageLoop(ClientClipboardPlugin* plugin);
        ~WinMessageLoop();
        void Start();
        void Stop();

        void OnDisplayDeviceChange();
    private:
        void CreateMessageWindow();
        void ThreadFunc();
        void OnWinSessionChange(uint32_t msg);
        static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
    private:
        ClientClipboardPlugin* plugin_ = nullptr;
        std::thread thread_;
        std::shared_ptr<WinMessageWindow> message_window_ = nullptr;
    };

}