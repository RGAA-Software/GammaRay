//
// Created by RGAA on 28/07/2025.
//

#ifndef GAMMARAYPREMIUM_GR_PANEL_GUARD_H
#define GAMMARAYPREMIUM_GR_PANEL_GUARD_H

#include <memory>
#include <vector>

namespace asio2
{
    class timer;
}

namespace tc
{

    class Thread;
    class ProcessInfo;
    class GrGuardContext;
    class MessageListener;

    class GrPanelGuard {
    public:
        explicit GrPanelGuard(const std::shared_ptr<GrGuardContext>& ctx);
        void Start();
        void Exit();

    private:
        bool CheckPanelState(const std::vector<std::shared_ptr<ProcessInfo>>& pss);
        void StartPanel();
        bool CheckSysInfoState(const std::vector<std::shared_ptr<ProcessInfo>>& pss);
        void StartSysInfo();

    private:
        std::shared_ptr<GrGuardContext> context_ = nullptr;
        std::shared_ptr<Thread> thread_ = nullptr;
        std::shared_ptr<asio2::timer> timer_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAYPREMIUM_GR_PANEL_GUARD_H
