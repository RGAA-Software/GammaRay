#include "win_panel_message_window.h"
#include <iostream>
#include <atomic>
#include "tc_common_new/log.h"
#include "win_panel_message_loop.h"
#include "render_panel/gr_context.h"

namespace tc
{

    constexpr char kWindowClassName[] = "GammaRay_render_MessageWindowClass";
    std::atomic<int> WinMessageWindow::current_create_window_count_ = 0;
    std::string WinMessageWindow::class_name_;
    std::atomic<bool> WinMessageWindow::class_registered_ = false;
    std::mutex WinMessageWindow::register_mutex_;

    std::shared_ptr<WinMessageWindow> WinMessageWindow::Make(const std::shared_ptr<GrContext>& ctx, std::shared_ptr<WinMessageLoop> message_loop) {
        return std::make_shared<WinMessageWindow>(ctx, message_loop);
    }

    WinMessageWindow::WinMessageWindow(const std::shared_ptr<GrContext>& ctx, std::shared_ptr<WinMessageLoop> message_loop) {
        context_ = ctx;
        message_loop_ = message_loop;
    }

    WinMessageWindow::~WinMessageWindow() {
        CloseWindow();
    }

    // static
    LRESULT CALLBACK WinMessageWindow::windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto self = reinterpret_cast<WinMessageWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));

        switch (msg)
        {
            // Set up the self before handling WM_CREATE.
        case WM_CREATE: {
            // lParam from function CreateWindowA(..., this)
            LPCREATESTRUCT cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = reinterpret_cast<WinMessageWindow*>(cs->lpCreateParams);

            // Make |hwnd| available to the message handler. At this point the
            // control hasn't returned from CreateWindow() yet.
            self->mHwnd = window;

            // Store pointer to the self to the window's user data.
            SetLastError(ERROR_SUCCESS);
            LONG_PTR result =
                SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            break;
        }
        case WM_CLOSE: {
            DestroyWindow(window);
            break;
        }

        // Clear the pointer to stop calling the self once WM_DESTROY is
        // received.
        case WM_DESTROY: {
            SetLastError(ERROR_SUCCESS);
            LONG_PTR result = SetWindowLongPtrA(window, GWLP_USERDATA, 0);
            PostQuitMessage(0);
            break;
        }

        case WM_CLIPBOARDUPDATE: {
            self->OnClipboardUpdate(window);
            break;
        }

        case WM_WTSSESSION_CHANGE: {
            if (wParam == WTS_CONSOLE_CONNECT)
            {
                LOGI("WTS_CONSOLE_CONNECT");
                //self->session_change_delegate_(WTS_CONSOLE_CONNECT);
            }
            if (wParam == WTS_CONSOLE_DISCONNECT)
            {
                LOGI("WTS_CONSOLE_DISCONNECT");
                //self->session_change_delegate_(WTS_CONSOLE_DISCONNECT);
            }
            if (wParam == WTS_REMOTE_CONNECT)
            {
                LOGI("WTS_REMOTE_CONNECT");
                //self->session_change_delegate_(WTS_REMOTE_CONNECT);
            }
            if (wParam == WTS_REMOTE_DISCONNECT)
            {
                LOGI("WTS_REMOTE_DISCONNECT");
                //self->session_change_delegate_(WTS_REMOTE_DISCONNECT);
            }
            if (wParam == WTS_SESSION_LOGON)
            {
                LOGI("WTS_SESSION_LOGON");
                //self->session_change_delegate_(WTS_SESSION_LOGON);
            }
            if (wParam == WTS_SESSION_LOGOFF)
            {
                LOGI("WTS_SESSION_LOGOFF");
                //self->session_change_delegate_(WTS_SESSION_LOGOFF);
            }
            if (wParam == WTS_SESSION_LOCK)
            {
                LOGI("WTS_SESSION_LOCK");
                //self->session_change_delegate_(WTS_SESSION_LOCK);
            }
            if (wParam == WTS_SESSION_UNLOCK)
            {
                LOGI("WTS_SESSION_UNLOCK");
                //self->session_change_delegate_(WTS_SESSION_UNLOCK);
            }
            if (wParam == WTS_SESSION_REMOTE_CONTROL)
            {
                LOGI("WTS_SESSION_REMOTE_CONTROL");
                //self->session_change_delegate_(WTS_SESSION_REMOTE_CONTROL);
            }
            if (wParam == WTS_SESSION_CREATE)
            {
                LOGI("WTS_SESSION_CREATE");
                //self->session_change_delegate_(WTS_SESSION_CREATE);
            }
            if (wParam == WTS_SESSION_TERMINATE)
            {
                LOGI("WTS_SESSION_TERMINATE");
                //self->session_change_delegate_(WTS_SESSION_TERMINATE);
            }
            break;
        }

        case WM_DISPLAYCHANGE: {
            LOGI("WM_DISPLAYCHANGE");
            self->OnDisplayChange();
            break;
        }


        default:
            break;
        }
        return DefWindowProcA(window, msg, wParam, lParam);
    }

    // static
    bool WinMessageWindow::registerWindowClass(HINSTANCE instance)
    {
        std::lock_guard<std::mutex> lck{register_mutex_};
        if (class_registered_)
            return true;

        WNDCLASSEXA window_class;
        memset(&window_class, 0, sizeof(window_class));
        window_class.cbSize = sizeof(window_class);
        static std::once_flag flag;
        std::call_once(flag, []() {
            DWORD pid = GetCurrentProcessId();
            class_name_ = kWindowClassName;
            class_name_ + "_" + std::to_string(pid);
        });
        window_class.lpszClassName = class_name_.c_str();
        window_class.hInstance = instance;
        window_class.lpfnWndProc = windowProc;

        if (!RegisterClassExA(&window_class))
        {
            std::cout << "RegisterClassExW failed GetLastError = " << GetLastError() << std::endl;
            return false;
        }

        class_registered_ = true;
        return true;
    }


    bool WinMessageWindow::Create(const std::string& window_name) {

        HINSTANCE instance = nullptr;
        window_name_ = window_name;
        if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<char*>(&windowProc),
            &instance))
        {
            std::cout << "GetModuleHandleExA failed" << std::endl;
            return false;
        }

        if (!registerWindowClass(instance))
            return false;

        mHwnd = CreateWindowA(
            kWindowClassName,
            window_name_.c_str(),
            0, 0, 0, 0, 0,
            nullptr,
            nullptr,
            instance,
            this);

        if (!mHwnd)
        {
            std::cout << "CreateWindowA failed" << std::endl;
            return false;
        }
        ++current_create_window_count_;
        return true;
    }

    HWND WinMessageWindow::GetHwnd() const {
        return mHwnd;
    }

    void WinMessageWindow::CloseWindow() {
        if (mHwnd) {
            PostMessage(mHwnd, WM_CLOSE, 0, 0);
        }
        --current_create_window_count_;
    }

    void WinMessageWindow::OnClipboardUpdate(HWND hwnd) {
        if (!message_loop_) {
            return;
        }
        message_loop_->OnClipboardUpdate(hwnd);
    }

    void WinMessageWindow::OnDisplayChange() {
        if (!message_loop_) {
            return;
        }
        message_loop_->OnDisplayDeviceChange();
    }

}