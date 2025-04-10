#include "win_message_loop.h"
#include <iostream>
#include <wtsapi32.h>
#include "tc_common_new/log.h"
#include "win_message_window.h"
#include "rd_context.h"
#include "plugins/plugin_manager.h"

namespace tc {

constexpr char kWindowName[] = "GammaRay_render_MessageWindow";


void CALLBACK WinMessageLoop::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (event == EVENT_SYSTEM_DESKTOPSWITCH)
	{
		std::cout << "Desktop switch event detected." << std::endl;		
	}
}

std::shared_ptr<WinMessageLoop> WinMessageLoop::Make(const std::shared_ptr<RdContext>& ctx) {
	return std::make_shared<WinMessageLoop>(ctx);
}

WinMessageLoop::WinMessageLoop(const std::shared_ptr<RdContext>& ctx) {
    context_ = ctx;
    plugin_mgr_ = context_->GetPluginManager();
	message_window_ = std::make_shared<WinMessageWindow>(ctx);

	/*message_window_->session_change_delegate_.connect(
		std::bind(&WinMessageLoop::OnWinSessionChange, this, std::placeholders::_1)
	);

	message_window_->display_change_delegate_.connect(
		std::bind(&WinMessageLoop::OnDisplayDeviceChange, this)
	);*/
}

WinMessageLoop::~WinMessageLoop() {
	
}

void WinMessageLoop::OnLocalClipboardUpdate(HWND hwnd) {
	/*CtxLocalClipboardUpdateMsg msg;
	msg.hwnd = hwnd;
	cloud_ctx_->SendAppMessage(msg);*/
}

void WinMessageLoop::OnWinSessionChange(uint32_t message) {
	/*CtxWinSessionChangeMsg msg;
	msg.session_msg = message;
	cloud_ctx_->SendAppMessage(msg);*/
}

void WinMessageLoop::OnDisplayDeviceChange() {
	/*CtxDisplayDeviceMsg msg;
	cloud_ctx_->SendAppMessage(msg);*/
}

void WinMessageLoop::Start() {
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

	AddClipboardFormatListener(hwnd); // use qt clibpoard
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
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnhookWinEvent(hEventHook);
}

}