#pragma once
#include <Windows.h>
#include <memory>
#include <thread>

namespace tc {

class WinMessageWindow;

// 监听windows消息
class WinMessageLoop : public std::enable_shared_from_this<WinMessageLoop> {
public:
	static std::shared_ptr<WinMessageLoop> Make();
	WinMessageLoop();
	~WinMessageLoop();
	void Start();
	void Stop();
private:
	void ThreadFunc();
	void OnLocalClipboardUpdate(HWND hwnd);
	void OnWinSessionChange(uint32_t msg);
	void OnDisplayDeviceChange();
	static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
private:
	std::thread thread_;
	std::shared_ptr<WinMessageWindow> message_window_ = nullptr;
};

}