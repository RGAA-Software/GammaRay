#include "win_desktop_manager.h"
#include "win_message_loop.h"


namespace tc {

std::shared_ptr<WinDesktopManager> WinDesktopManager::Make() {
	return std::make_shared<WinDesktopManager>();
}

WinDesktopManager::WinDesktopManager() {
	message_loop_ = WinMessageLoop::Make();
	if(message_loop_) {
		message_loop_->Start();
	}	
}

WinDesktopManager::~WinDesktopManager() {
	if (message_loop_) {
		message_loop_->Stop();
	}
}

}
