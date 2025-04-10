#pragma once
#include <memory>

namespace tc
{

    class RdContext;
    class WinMessageLoop;

    class WinDesktopManager {
    public:
        static std::shared_ptr<WinDesktopManager> Make(const std::shared_ptr<RdContext>& ctx);
        explicit WinDesktopManager(const std::shared_ptr<RdContext>& ctx);
        ~WinDesktopManager();
    public:
    private:
        std::shared_ptr<RdContext> context_ = nullptr;
        std::shared_ptr<WinMessageLoop> message_loop_;
    };

}