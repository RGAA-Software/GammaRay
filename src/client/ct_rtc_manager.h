//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_CT_RTC_MANAGER_H
#define GAMMARAY_CT_RTC_MANAGER_H

#include <memory>
#include <QLibrary>
#include <QApplication>
#include "sdk_messages.h"

namespace tc
{

    class Thread;
    class ClientContext;
    class RtcClientInterface;
    class Message;
    class MessageListener;
    class Settings;
    class ThunderSdk;

    class CtRtcManager {
    public:
        explicit CtRtcManager(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk);

        void Init();

    private:
        void LoadRtcLibrary();
        void OnRemoteSdp(const SdkMsgRemoteAnswerSdp& m);
        void OnRemoteIce(const SdkMsgRemoteIce& m);

        void SendSdpToRemote(const std::string& sdp);
        void SendIceToRemote(const std::string& ice, const std::string& mid, int sdp_mline_index);

        void RunInRtcThread(std::function<void()>&&);

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<Thread> thread_ = nullptr;
        QLibrary* rtc_lib_ = nullptr;
        RtcClientInterface* rtc_client_ = nullptr;
        std::shared_ptr<MessageListener> msg_listener_ = nullptr;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
    };

}

#endif //GAMMARAY_CT_RTC_MANAGER_H
