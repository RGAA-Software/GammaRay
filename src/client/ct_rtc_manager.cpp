//
// Created by RGAA on 13/04/2025.
//

#include "ct_rtc_manager.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/log.h"
#include "webrtc/rtc_client_interface.h"
#include "ct_client_context.h"
#include "tc_common_new/message_notifier.h"
#include "tc_client_sdk_new/sdk_messages.h"
#include "ct_settings.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    CtRtcManager::CtRtcManager(const std::shared_ptr<ClientContext>& ctx) {
        context_ = ctx;
        settings_ = Settings::Instance();
        msg_listener_ = context_->ObtainMessageListener();
        thread_ = Thread::Make("rtc_client_thread", 1024);
        thread_->Poll();

        msg_listener_->Listen<SdkMsgNetworkConnected>([=, this](const SdkMsgNetworkConnected& msg) {
            LOGI("Sdk msg, network connected.");
        });

        msg_listener_->Listen<SdkMsgRoomPrepared>([=, this](const SdkMsgRoomPrepared& msg) {
            LOGI("Sdk msg, room prepared, enable p2p: {}", settings_->enable_p2p_);
            if (settings_->enable_p2p_) {
                this->Init();
            }
        });

        msg_listener_->Listen<SdkMsgRemoteAnswerSdp>([=, this](const SdkMsgRemoteAnswerSdp& msg) {

        });

        msg_listener_->Listen<SdkMsgRemoteIce>([=, this](const SdkMsgRemoteIce& msg) {

        });

        this->LoadRtcLibrary();
    }

    void CtRtcManager::Init() {
        thread_->Post([=, this]() {
            if (!rtc_client_) {
                return;
            }
            if (!rtc_client_->Init()) {
                LOGE("RTC client init FAILED!");
                return;
            }

            LOGI("RTC client init success");
        });
    }

    void CtRtcManager::LoadRtcLibrary() {
        thread_->Post([=, this]() {
            auto lib_name = QApplication::applicationDirPath() + "/gr_client/tc_rtc_client.dll";
            rtc_lib_ = new QLibrary(lib_name);
            auto r = rtc_lib_->load();
            if (!r) {
                LOGE("LOAD rtc conn FAILED");
                return;
            }

            auto fn_get_instance = (FnGetInstance)rtc_lib_->resolve("GetInstance");
            if (!fn_get_instance) {
                LOGE("DON'T have GetInstance");
                return;
            }

            rtc_client_ = (RtcClientInterface*)fn_get_instance();
            if (!rtc_client_) {
                LOGE("Can't get rtc client instance.");
            }
        });
    }
}