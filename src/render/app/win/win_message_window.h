#pragma once

#include <Windows.h>
#include <functional>
#include <atomic>
#include <mutex>

//#include <boost/signals2/signal.hpp>

namespace tc
{

    class RdContext;
    class PluginManager;

    class WinMessageWindow {
    public:
        explicit WinMessageWindow(const std::shared_ptr<RdContext>& ctx);
        ~WinMessageWindow();
        using MessageCallback = std::function<bool(UINT message,
            WPARAM wparam,
            LPARAM lparam,
            LRESULT& result)>;
        bool Create(const std::string& window_name);
        HWND GetHwnd() const;
        void CloseWindow();

        void OnClipboardUpdate(HWND hwnd);

        /*剪切板相关消息*/
        //boost::signals2::signal<void(HWND)> clipboadr_monitor_delegate;

        /*显示设备变化消息*/
        //boost::signals2::signal<void()> display_change_delegate_;
    private:
        static bool registerWindowClass(HINSTANCE instance);
        static LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<PluginManager> plugin_mgr_ = nullptr;
        MessageCallback message_callback_;
        HWND mHwnd = nullptr;
        std::string window_name_;

        static std::mutex register_mutex_;
        static std::atomic<int> current_create_window_count_;
        static std::string class_name_;
        static std::atomic<bool> class_registered_;
    };
}