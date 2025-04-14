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
#include "tc_client_sdk_new/thunder_sdk.h"
#include "ct_settings.h"
#include "tc_message.pb.h"

typedef void *(*FnGetInstance)();

namespace tc
{

    CtRtcManager::CtRtcManager(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk) {
        context_ = ctx;
        sdk_ = sdk;
        settings_ = Settings::Instance();
        msg_listener_ = context_->ObtainMessageListener();
        thread_ = Thread::Make("rtc_client_thread", 1024);
        thread_->Poll();

        msg_listener_->Listen<SdkMsgNetworkConnected>([=, this](const SdkMsgNetworkConnected& msg) {
            LOGI("Sdk msg, network connected.");
        });

        msg_listener_->Listen<SdkMsgRoomPrepared>([=, this](const SdkMsgRoomPrepared& msg) {
            if (settings_->enable_p2p_ && msg.room_type_ == kRoomTypeMedia) {
                LOGI("Sdk msg, room prepared, will init webrtc!");
                this->Init();
            }
        });

        msg_listener_->Listen<SdkMsgRemoteAnswerSdp>([=, this](const SdkMsgRemoteAnswerSdp& msg) {
            this->OnRemoteSdp(msg);
        });

        msg_listener_->Listen<SdkMsgRemoteIce>([=, this](const SdkMsgRemoteIce& msg) {
            this->OnRemoteIce(msg);
        });

        this->LoadRtcLibrary();
    }

    void CtRtcManager::Init() {
        RunInRtcThread([=, this]() {
            if (!rtc_client_) {
                return;
            }
            if (!rtc_client_->Init()) {
                LOGE("RTC client init FAILED!");
                return;
            }

            rtc_client_->SetOnLocalSdpSetCallback([=, this](const std::string& sdp) {
                // TODO: send to remote.
                this->SendSdpToRemote(sdp);
            });

            rtc_client_->SetOnLocalIceCallback([=, this](const std::string& ice, const std::string& mid, int sdp_mline_index) {
                // TODO: send to remote
                this->SendIceToRemote(ice, mid, sdp_mline_index);
            });

            LOGI("RTC client init success");
        });
    }

    void CtRtcManager::LoadRtcLibrary() {
        RunInRtcThread([=, this]() {
            LOGI("Begin to load library!");
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
                return;
            }
            LOGI("Load Rtc library success.");
        });
    }

    void CtRtcManager::OnRemoteSdp(const SdkMsgRemoteAnswerSdp& m) {
        RunInRtcThread([=, this]() {
            if (rtc_client_) {
                rtc_client_->OnRemoteSdp(m.answer_sdp_.sdp());
            }
        });
    }

    void CtRtcManager::OnRemoteIce(const SdkMsgRemoteIce& m) {
        RunInRtcThread([=, this]() {
            if (rtc_client_) {
                auto sub = m.ice_;
                rtc_client_->OnRemoteIce(sub.ice(), sub.mid(), sub.sdp_mline_index());
            }
        });
    }

    void CtRtcManager::RunInRtcThread(std::function<void()>&& task) {
        thread_->Post([=]() {
            task();
        });
    }

    void CtRtcManager::SendSdpToRemote(const std::string& sdp) {
        // pack to proto & send it
        tc::Message pt_msg;
        pt_msg.set_device_id(settings_->device_id_);
        pt_msg.set_stream_id(settings_->stream_id_);
        pt_msg.set_type(tc::MessageType::kSigOfferSdpMessage);
        auto sub = pt_msg.mutable_sig_offer_sdp();
        sub->set_device_id(settings_->device_id_);
        sub->set_sdp(sdp);
        sdk_->PostMediaMessage(pt_msg.SerializeAsString());
    }

    void CtRtcManager::SendIceToRemote(const std::string& ice, const std::string& mid, int sdp_mline_index) {
        // pack to proto & send it
        tc::Message pt_msg;
        pt_msg.set_device_id(settings_->device_id_);
        pt_msg.set_stream_id(settings_->stream_id_);
        pt_msg.set_type(tc::MessageType::kSigIceMessage);
        auto sub = pt_msg.mutable_sig_ice();
        sub->set_device_id(settings_->device_id_);
        sub->set_ice(ice);
        sub->set_mid(mid);
        sub->set_sdp_mline_index(sdp_mline_index);
        sdk_->PostMediaMessage(pt_msg.SerializeAsString());
    }
}