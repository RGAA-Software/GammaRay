//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_CT_RTC_MANAGER_H
#define GAMMARAY_CT_RTC_MANAGER_H

#include <memory>
#include <QLibrary>
#include <QApplication>

namespace tc
{

    class Thread;
    class ClientContext;
    class RtcClientInterface;
    class Message;
    class MessageListener;
    class Settings;

    class CtRtcManager {
    public:
        explicit CtRtcManager(const std::shared_ptr<ClientContext>& ctx);

        void Init();

    private:
        void LoadRtcLibrary();

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<Thread> thread_ = nullptr;
        QLibrary* rtc_lib_ = nullptr;
        RtcClientInterface* rtc_client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
    };

}

#endif //GAMMARAY_CT_RTC_MANAGER_H
