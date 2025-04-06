#pragma once
#include <memory>
namespace tc {

class WinMessageLoop;

class WinDesktopManager {
public:
	static std::shared_ptr<WinDesktopManager> Make();
	WinDesktopManager();
	~WinDesktopManager();
public:
private:
	std::shared_ptr<WinMessageLoop> message_loop_;
};

}