//
// Created by hy on 2024/4/25.
//

#include "rtc_server.h"
#include "peer_callback.h"
#include "desktop_capture.h"
#include "desktop_capture_source.h"

using namespace webrtc;

namespace tc
{

    std::shared_ptr<RtcServer> RtcServer::Make(const std::shared_ptr<RtcContext>& ctx) {
        return std::make_shared<RtcServer>(ctx);
    }

    RtcServer::RtcServer(const std::shared_ptr<RtcContext>& ctx) {
        rtc_ctx_ = ctx;
    }

    bool RtcServer::Start() {
        webrtc::field_trial::InitFieldTrialsFromString("");
        rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
        rtc::InitializeSSL();

        peer_callback_ = PeerCallback::Make(shared_from_this());
        set_sess_callback_ = SetSessCallback::Make(shared_from_this());
        create_sess_callback_ = CreateSessCallback::Make(shared_from_this());
        CreatePeerConnectionFactory();
        CreatePeerConnection();
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

    void RtcServer::CreatePeerConnectionFactory() {
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
                nullptr, nullptr);

        if (peer_conn_factory_.get() == nullptr) {
            std::cout << ":" << std::this_thread::get_id() << ":" << "Error on CreateModularPeerConnectionFactory." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "after init ...." << std::endl;
    }

    void RtcServer::CreatePeerConnection() {
        configuration_.port_allocator_config.min_port = 30000;
        configuration_.port_allocator_config.max_port = 30200;
        auto result = peer_conn_factory_->
                CreatePeerConnectionOrError(configuration_, webrtc::PeerConnectionDependencies(peer_callback_.get()));
        if (!result.ok()) {
            std::cerr << "create peer connection failed: " << result.error().message() << std::endl;
            return;
        }
        std::cout << "after create peer connection" << std::endl;
        auto peer_conn = result.value();

        if (peer_conn.get() == nullptr) {
            peer_conn_factory_ = nullptr;
            std::cout << ":" << std::this_thread::get_id() << ":" << "Error on CreatePeerConnection." << std::endl;
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

        desktop_capture_ = DesktopCapture::Create(60, 0);
        desktop_capture_->StartCapture();

        mock_video_source_ = std::make_shared<VideoSourceMock>();

        video_track_ = new rtc::RefCountedObject<VideoTrack>(desktop_capture_);
        auto video_track = peer_conn_factory_->CreateVideoTrack(video_track_, "video");
        peer_conn_->AddTrack(video_track, {"stream1"});

    }

    void RtcServer::OnSessionCreated(webrtc::SessionDescriptionInterface *desc) {
        peer_conn_->SetLocalDescription(this->set_sess_callback_.get(), desc);
        std::string sdp;
        desc->ToString(&sdp);
        this->sdp_ = sdp;

    }

    void RtcServer::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        std::string ice_candidate;
        candidate->ToString(&ice_candidate);
        std::string ice_sdp_mid = candidate->sdp_mid();
        int ice_sdp_mline_idx = candidate->sdp_mline_index();


    }

    void RtcServer::OnIceGatheringComplete() {

    }

    void RtcServer::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {

    }

    void RtcServer::Exit() {
        if (peer_conn_) {
            peer_conn_->Close();
            peer_conn_ = nullptr;
        }
        peer_conn_factory_ = nullptr;

        if (network_thread_) {
            network_thread_->Stop();
        }
        if (worker_thread_) {
            worker_thread_->Stop();
        }
        if (sig_thread_) {
            sig_thread_->Stop();
        }

        rtc::CleanupSSL();
    }

}
