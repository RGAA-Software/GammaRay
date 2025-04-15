//
// Created by RGAA on 13/04/2025.
//

#include "rtc_connection.h"
#include "tc_common_new/log.h"
#include "tc_common_new/webrtc_helper.h"
#include "tc_common_new/time_util.h"
#include "peer_callback.h"
#include "rtc_data_channel.h"
#include <QApplication>

void* GetInstance() {
    static tc::RtcConnection conn;
    return (void*)&conn;
}

using namespace webrtc;

namespace tc
{

    RtcConnection::RtcConnection() {

    }

    bool RtcConnection::Init() {
        auto beg = TimeUtil::GetCurrentTimestamp();
        auto exe_dir = qApp->applicationDirPath();
        Logger::InitLog(exe_dir.toStdString() + "/gr_logs/client_rtc.log", true);
        LOGI("*******************************");
        LOGI("========BEGIN RTC INIT=========");

        webrtc::field_trial::InitFieldTrialsFromString("");
        rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
        rtc::InitializeSSL();

        peer_callback_ = PeerCallback::Make(this);
        set_local_sdp_callback_ = SetSessCallback::Make(this);
        set_remote_sdp_callback_ = SetSessCallback::Make(this);
        create_sess_callback_ = CreateSessCallback::Make(this);

        // create sess callback
        create_sess_callback_->SetOnCreateSdpSuccessCallback([=, this](webrtc::SessionDescriptionInterface* desc) {
            LOGI("Create sdp success, will set local desc.");
            std::string sdp;
            if (!desc->ToString(&sdp)) {
                LOGE("Convert to sdp string failed.");
                return;
            }
            local_sdp_ = sdp;
            // set local description
            peer_conn_->SetLocalDescription(set_local_sdp_callback_.get(), desc);
        });

        create_sess_callback_->SetOnCreateSdpFailedCallback([=, this](const std::string& msg) {
            LOGE("Create sdp failed: {}", msg);
        });

        // set local sdp callback
        set_local_sdp_callback_->SetSdpSuccessCallback([=, this]() {
            LOGI("Set local desc success, will send to remote.");
            if (local_sdp_set_cbk_) {
                local_sdp_set_cbk_(local_sdp_);
            }
        });

        set_local_sdp_callback_->SetSdpFailedCallback([=, this](const std::string& msg) {
            LOGI("Set local desc failed: {}", msg);
        });

        // set remote sdp callback
        set_remote_sdp_callback_->SetSdpSuccessCallback([=, this]() {
            LOGI("Set remote desc success.");

            already_set_answer_sdp_ = true;

            // cached ice ?
            this->SendCachedIces();
        });

        set_remote_sdp_callback_->SetSdpFailedCallback([=, this](const std::string& msg) {
            LOGI("Set remote desc failed: {}", msg);
        });

        // PeerConnection
        peer_callback_->SetOnIceCallback([=, this](const std::string& ice, const std::string& mid, int sdp_mline_idx) {
            // TODO
            if (already_set_answer_sdp_) {
                // send it
                std::lock_guard<std::mutex> guard(ice_mtx_);
                if (local_ice_cbk_) {
                    local_ice_cbk_(ice, mid, sdp_mline_idx);
                }
            }
            else {
                // cache it
                cached_ices_.push_back(CachedIce {
                    .ice_ = ice,
                    .mid_ = mid,
                    .sdp_mline_index_ = sdp_mline_idx,
                });
            }
        });

        CreatePeerConnectionFactory();
        CreatePeerConnection();
        auto end = TimeUtil::GetCurrentTimestamp();
        LOGI("========AFTER RTC INIT=========, used: {}ms", (end-beg));
        return true;
    }

    static void CreateSomeMediaDeps(PeerConnectionFactoryDependencies& media_deps) {
        media_deps.adm = AudioDeviceModule::CreateForTest(
                AudioDeviceModule::kDummyAudio, media_deps.task_queue_factory.get());
        media_deps.audio_encoder_factory =
                webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
        media_deps.audio_decoder_factory =
                webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>();
        media_deps.video_encoder_factory =
                std::make_unique<VideoEncoderFactoryTemplate<
                                 LibvpxVp8EncoderTemplateAdapter, LibvpxVp9EncoderTemplateAdapter,
                        OpenH264EncoderTemplateAdapter, LibaomAv1EncoderTemplateAdapter>>();
        media_deps.video_decoder_factory =
                std::make_unique<VideoDecoderFactoryTemplate<
                                 LibvpxVp8DecoderTemplateAdapter, LibvpxVp9DecoderTemplateAdapter,
                        OpenH264DecoderTemplateAdapter, Dav1dDecoderTemplateAdapter>>();
        media_deps.audio_processing = webrtc::AudioProcessingBuilder().Create();
    }

    void RtcConnection::CreatePeerConnectionFactory() {
        configuration_.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
        configuration_.media_config.video.periodic_alr_bandwidth_probing = true;
        //configuration_.enable_dtls_srtp = true;

        {
            auto stun = webrtc::PeerConnectionInterface::IceServer();
            stun.uri = "stun:39.91.109.105:60498";
            configuration_.servers.push_back(stun);
        }
        if (false) {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.tls_cert_policy = webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
            turn.uri = "turn:xxxx.xyz:3478";
            turn.username = "test";
            turn.password = "123456";
            configuration_.servers.push_back(turn);
        }

        network_thread_ = rtc::Thread::CreateWithSocketServer();
        network_thread_->Start();
        worker_thread_ = rtc::Thread::Create();
        worker_thread_->Start();
        sig_thread_ = rtc::Thread::Create();
        sig_thread_->Start();

        webrtc::PeerConnectionFactoryDependencies media_deps;
        media_deps.task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
        CreateSomeMediaDeps(media_deps);

        peer_conn_factory_ = webrtc::CreatePeerConnectionFactory(
            network_thread_.get(), worker_thread_.get(), sig_thread_.get(),
            nullptr,
            std::move(media_deps.audio_encoder_factory),
            std::move(media_deps.audio_decoder_factory),
            std::move(media_deps.video_encoder_factory),
            std::move(media_deps.video_decoder_factory),
            nullptr, nullptr);

        if (peer_conn_factory_.get() == nullptr) {
            LOGE("Error on CreateModularPeerConnectionFactory.");
            //exit(EXIT_FAILURE);
            // TODO:
        }
    }

    void RtcConnection::CreatePeerConnection() {
        configuration_.port_allocator_config.min_port = 60430;
        configuration_.port_allocator_config.max_port = 60490;
        auto result = peer_conn_factory_->
                CreatePeerConnectionOrError(configuration_, webrtc::PeerConnectionDependencies(peer_callback_.get()));
        if (!result.ok()) {
            LOGE("create peer connection failed: {}", result.error().message());
            return;
        }
        LOGI("after create peer connection");
        auto peer_conn = result.value();

        if (peer_conn.get() == nullptr) {
            peer_conn_factory_ = nullptr;
            LOGE("Error on CreatePeerConnection.");
            //exit(EXIT_FAILURE);
            // TODO:
        }
        this->peer_conn_ = peer_conn;

        {
            webrtc::DataChannelInit data_channel_config;
            data_channel_config.ordered = true;
            RTCErrorOr<rtc::scoped_refptr<DataChannelInterface>> r_dc = peer_conn->CreateDataChannelOrError(
                    "media_data_channel", &data_channel_config);
            if (!r_dc.ok()) {
                LOGE("create datachannel error: {}", r_dc.error().message());
            } else {
                media_data_channel_ = RtcDataChannel::Make(this, r_dc.value());
            }
        }
        {
            webrtc::DataChannelInit data_channel_config;
            data_channel_config.ordered = true;
            RTCErrorOr<rtc::scoped_refptr<DataChannelInterface>> r_dc = peer_conn->CreateDataChannelOrError(
                    "ft_data_channel", &data_channel_config);
            if (!r_dc.ok()) {
                LOGE("create datachannel error: {}", r_dc.error().message());
            } else {
                ft_data_channel_ = RtcDataChannel::Make(this, r_dc.value());
            }
        }

        auto options = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
        options.offer_to_receive_audio = true;
        options.offer_to_receive_video = true;
        //auto create_session_observer = this->peer_callback_.get();
        peer_conn->CreateOffer(create_sess_callback_.get(), options);

        std::string typeStr = "offer";
        absl::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(typeStr);
        if (!type_maybe) {

        }

    }

    bool RtcConnection::OnRemoteSdp(const std::string &sdp) {
        LOGI("OnRemoteSdp, Will set remote answer sdp");
        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("answer", sdp, &error));
        peer_conn_->SetRemoteDescription(set_remote_sdp_callback_.get(), session_description);
        already_set_answer_sdp_ = error.line.empty();
        if (already_set_answer_sdp_) {
            LOGI("SetRemoteAnswerSdp success, may send ices.");
            //this->SendCachedIces();
        } else {
            LOGE("SetRemoteAnswerSdp failed, error line: {}, desc: {}", error.line, error.description);
            return false;
        }
        return true;
    }

    bool RtcConnection::OnRemoteIce(const std::string &ice, const std::string &mid, int32_t sdp_mline_index) {
        LOGI("OnRemoteIce, Will set remote ice: {}", ice);
        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::IceCandidateInterface>
                candidate(webrtc::CreateIceCandidate(mid, sdp_mline_index, ice, &error));
        if (!error.line.empty()) {
            LOGE("Create IceCandidate failed: {} - {}", error.line, error.description);
            return false;
        }
        peer_conn_->AddIceCandidate(std::move(candidate), [](webrtc::RTCError error) {
            if (error.ok()) {
                LOGI("AddIceCandidate success.");
            } else {
                LOGE("AddIceCandidate failed: {}", error.message());
            }
        });
        return false;
    }

    void RtcConnection::SendCachedIces() {
        if (local_ice_cbk_) {
            std::lock_guard<std::mutex> guard(ice_mtx_);
            for (const auto& ci : cached_ices_) {
                local_ice_cbk_(ci.ice_, ci.mid_, ci.sdp_mline_index_);
            }
        }
    }

}