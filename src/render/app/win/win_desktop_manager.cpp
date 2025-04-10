#include "win_desktop_manager.h"
#include "win_message_loop.h"

namespace tc
{

    std::shared_ptr<WinDesktopManager> WinDesktopManager::Make(const std::shared_ptr<RdContext>& ctx) {
        return std::make_shared<WinDesktopManager>(ctx);
    }

    WinDesktopManager::WinDesktopManager(const std::shared_ptr<RdContext>& ctx) {
        message_loop_ = WinMessageLoop::Make(ctx);
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
