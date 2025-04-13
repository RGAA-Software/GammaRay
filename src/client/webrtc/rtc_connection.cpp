//
// Created by RGAA on 13/04/2025.
//

#include "rtc_connection.h"
#include "tc_common_new/log.h"
#include "tc_common_new/webrtc_helper.h"
#include "tc_common_new/time_util.h"
#include "peer_callback.h"
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
        LOGI("========BEGIN RTC INIT=========");

        webrtc::field_trial::InitFieldTrialsFromString("");
        rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
        rtc::InitializeSSL();

        peer_callback_ = PeerCallback::Make(this);
        set_sess_callback_ = SetSessCallback::Make(this);
        create_sess_callback_ = CreateSessCallback::Make(this);

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
            stun.uri = "stun:syxmsg.xyz:3478";
            configuration_.servers.push_back(stun);
        }
        {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.tls_cert_policy = webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
            turn.uri = "turn:syxmsg.xyz:3478";
            turn.username = "test";
            turn.password = "123456";
            configuration_.servers.push_back(turn);
        }

        {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.tls_cert_policy = webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
            turn.uri = "turn:syxmsg.xyz:3478?transport=tcp";
            turn.username = "test";
            turn.password = "123456";
            configuration_.servers.push_back(turn);
        }
        {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.uri = "turn:syxmsg.xyz:3478?transport=udp";
            turn.username = "test";
            turn.password = "123456";
            configuration_.servers.push_back(turn);
        }
        {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.tls_cert_policy = webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
            turn.uri = "turn:syxmsg.xyz:5349?transport=tcp";
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
                //webrtc::CreateBuiltinAudioEncoderFactory(),
                //webrtc::CreateBuiltinAudioDecoderFactory(),
                //webrtc::CreateBuiltinVideoEncoderFactory(),
                //webrtc::CreateBuiltinVideoDecoderFactory(),
                nullptr, nullptr);

        if (peer_conn_factory_.get() == nullptr) {
            LOGE("Error on CreateModularPeerConnectionFactory.");
            exit(EXIT_FAILURE);
        }
    }

    void RtcConnection::CreatePeerConnection() {
        configuration_.port_allocator_config.min_port = 30000;
        configuration_.port_allocator_config.max_port = 30200;
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
            exit(EXIT_FAILURE);
        }
        this->peer_conn_ = peer_conn;
#if 0
        auto options = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
        options.offer_to_receive_audio = true;
        options.offer_to_receive_video = true;
        auto create_session_observer = this->peer_callback_.get();
        peer_conn->CreateOffer(create_session_observer, options);

        std::string typeStr = "offer";
        absl::optional<webrtc::SdpType> type_maybe = webrtc::SdpTypeFromString(typeStr);
        if (!type_maybe) {

        }
#endif

//        desktop_capture_ = DesktopCapture::Create(60, 0);
//        desktop_capture_->StartCapture();
//
//        mock_video_source_ = std::make_shared<VideoSourceMock>();
//
//        video_track_ = new rtc::RefCountedObject<VideoTrack>(desktop_capture_);
//        auto video_track = peer_conn_factory_->CreateVideoTrack(video_track_, "video");
//        peer_conn_->AddTrack(video_track, {"stream1"});

    }

}