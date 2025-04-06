#pragma once

#include <Windows.h>
#include <functional>
#include <atomic>
#include <mutex>

//#include <boost/signals2/signal.hpp>

namespace tc {
class WinMessageWindow {
public:
	WinMessageWindow() = default;
	~WinMessageWindow();
	using MessageCallback = std::function<bool(UINT message,
		WPARAM wparam,
		LPARAM lparam,
		LRESULT& result)>;
	bool Create(const std::string& window_name);
	HWND GetHwnd() const;
	void CloseWindow();
	/*剪切板相关消息*/
	//boost::signals2::signal<void(HWND)> clipboadr_monitor_delegate;

	/*windows 会话消息变更 目前应用与控制台会话监测*/
	//boost::signals2::signal<void(uint32_t)> session_change_delegate_;

	/*显示设备变化消息*/
	//boost::signals2::signal<void()> display_change_delegate_;
private:
	

	static bool registerWindowClass(HINSTANCE instance);
	static LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
	MessageCallback message_callback_;
	HWND mHwnd = nullptr;
	std::string window_name_;

	static std::mutex register_mutex_;
	static std::atomic<int> current_create_window_count_;
	static std::string class_name_;
	static std::atomic<bool> class_registered_;
};
}