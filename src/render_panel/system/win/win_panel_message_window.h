#pragma once

#include <Windows.h>
#include <functional>
#include <atomic>
#include <mutex>

namespace tc
{

    class GrContext;
    class WinMessageLoop;

    using MessageCallback = std::function<bool(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result)>;

    class WinMessageWindow {
    public:
        static std::shared_ptr<WinMessageWindow> Make(const std::shared_ptr<GrContext>& ctx, std::shared_ptr<WinMessageLoop> message_loop);
        explicit WinMessageWindow(const std::shared_ptr<GrContext>& ctx, std::shared_ptr<WinMessageLoop> message_loop);
        ~WinMessageWindow();
        bool Create(const std::string& window_name);
        HWND GetHwnd() const;
        void CloseWindow();
    private:
        static bool registerWindowClass(HINSTANCE instance);
        static LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

        /*剪切板更新*/
        void OnClipboardUpdate(HWND hwnd);

        /*显示设备变化消息*/
        void OnDisplayChange();

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        MessageCallback message_callback_;
        HWND mHwnd = nullptr;
        std::string window_name_;

        std::shared_ptr<WinMessageLoop> message_loop_;

        static std::mutex register_mutex_;
        static std::atomic<int> current_create_window_count_;
        static std::string class_name_;
        static std::atomic<bool> class_registered_;
    };
}