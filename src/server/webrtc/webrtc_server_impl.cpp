#include "webrtc_server_impl.h"

#include "observers/peer_conn_observer_impl.h"
#include "observers/set_session_desc_observer_impl.h"
#include "observers/create_session_desc_observer_impl.h"
#include "self_gen_video_track.h"
#include "h264_encoder_factory.h"
#include "tc_common_new/image.h"
#include "tc_common_new/log.h"

namespace tc
{

    std::shared_ptr<WebRtcServerImpl> WebRtcServerImpl::Make() {
        return std::make_shared<WebRtcServerImpl>();
    }

    WebRtcServerImpl::WebRtcServerImpl() {

    }

    void WebRtcServerImpl::Init(const WebRtcServerParam& param) {
        client_param_ = param;
        webrtc::field_trial::InitFieldTrialsFromString("");
        rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
        rtc::InitializeSSL();

        peer_conn_observer_ = PeerConnObserverImpl::Make(shared_from_this());
        set_session_desc_observer_ = SetSessionDescObserverImpl::Make(shared_from_this());
        create_session_desc_observer_ = CreateSessionDescObserverImpl::Make(shared_from_this());
        video_frame_observer_ = VideoFrameObserver::Make(RtcDecodedVideoFrame::RtcFrameType::kRtcFrameRGB24, param.frame_callback_);

        InitPeerConnectionFactory();
        InitPeerConnection();
    }

    void WebRtcServerImpl::Exit() {
        peer_conn_->Close();
        peer_conn_ = nullptr;
        peer_connection_factory_    = nullptr;

        network_thread_->Stop();
        worker_thread_->Stop();
        signaling_thread_->Stop();

        rtc::CleanupSSL();
    }

    std::shared_ptr<PeerConnObserverImpl> WebRtcServerImpl::GetPeerConnObserver() {
        return peer_conn_observer_;
    }

    std::shared_ptr<VideoFrameObserver> WebRtcServerImpl::GetVideoFrameObserver() {
        return video_frame_observer_;
    }

    rtc::scoped_refptr<SetSessionDescObserverImpl> WebRtcServerImpl::GetSetSessionDescObserver() {
        return set_session_desc_observer_;
    }

    rtc::scoped_refptr<CreateSessionDescObserverImpl> WebRtcServerImpl::GetCreateSessionDescObserver() {
        return create_session_desc_observer_;
    }

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> WebRtcServerImpl::GetPeerConnection() {
        return peer_conn_;
    }

    void WebRtcServerImpl::SetPeerConnection(const rtc::scoped_refptr<webrtc::PeerConnectionInterface>& pc) {
        peer_conn_ = pc;
    }

    // ---- callbacks ---- //

    void WebRtcServerImpl::OnSessionCreated(webrtc::SessionDescriptionInterface *desc) {
        peer_conn_->SetLocalDescription(set_session_desc_observer_.get(), desc);
        std::string sdp;
        desc->ToString(&sdp);
        this->sdp_ = sdp;
        std::cout << "sdp: " << sdp << std::endl;
    }

    void WebRtcServerImpl::OnIceCandidate(const webrtc::IceCandidateInterface *candidate) {
        std::string ice_candidate;
        std::string ice_sdp_mid;
        int ice_sdp_mline_idx;
        candidate->ToString(&ice_candidate);
        ice_sdp_mid = candidate->sdp_mid();
        ice_sdp_mline_idx = candidate->sdp_mline_index();
        std::cout << "mid: " << ice_sdp_mid << ", mline idx: " << ice_sdp_mline_idx << ", candidate: " << ice_candidate << std::endl;
    }

    void WebRtcServerImpl::OnIceGatheringComplete() {
        //this->RequestRemoteSDP();
    }

    void WebRtcServerImpl::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
        auto track = transceiver->receiver()->track();
        if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
            auto video_track = static_cast<webrtc::VideoTrackInterface*>(track.get());
            video_track->AddOrUpdateSink(video_frame_observer_.get(), rtc::VideoSinkWants());
            std::cout << "AddOrUpdateSink..." << std::endl;
        }
    }

    void WebRtcServerImpl::RequestRemoteSDP() {
        json body_obj {
                {"session_id", "ss1"},
                {"offer", sdp_}
        };
        auto body = body_obj.dump();
        httplib::Headers headers = {
                {"Content-Type", "application/json"}
        };

        httplib::Client cli(client_param_.remote_ip_, client_param_.port_);
        auto res = cli.Post("/public/getRemoteConn", headers, body, "application/json");
        std::cout << "request sdp status: " << res->status << std::endl;

        if (res && res->status == 200) {
            std::cout << "body: " << res->body << std::endl;
            std::string remote_sdp;
            try {
                auto obj = json::parse(res->body);
                if (obj["code"].get<int>() != 0) {
                    return;
                }
                remote_sdp = obj["result"]["answer"].get<std::string>();
                if (remote_sdp.empty()) {
                    return;
                }
            } catch(std::exception& e) {
                std::cerr << "parse response failed: " << e.what() << std::endl;
                return;
            }

            webrtc::SdpParseError error;
            webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("answer", remote_sdp, &error));
            peer_conn_->SetRemoteDescription(set_session_desc_observer_.get(), session_description);
        } else {
            std::cout << "Failed to make request  " << res.error() << std::endl;
        }
    }

    void WebRtcServerImpl::CreateSomeMediaDeps(webrtc::PeerConnectionFactoryDependencies& media_deps) {
//        media_deps.adm = nullptr;//webrtc::AudioDeviceModule::CreateForTest(webrtc::AudioDeviceModule::kDummyAudio, media_deps.task_queue_factory.get());
//        media_deps.audio_encoder_factory = webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>();
//        media_deps.audio_decoder_factory = webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>();
//        media_deps.video_encoder_factory = std::make_unique<webrtc::VideoEncoderFactoryTemplate<
//                        webrtc::LibvpxVp8EncoderTemplateAdapter, webrtc::LibvpxVp9EncoderTemplateAdapter,
//                        webrtc::OpenH264EncoderTemplateAdapter, webrtc::LibaomAv1EncoderTemplateAdapter>>();
//        media_deps.video_decoder_factory = std::make_unique<webrtc::VideoDecoderFactoryTemplate<
//                        webrtc::LibvpxVp8DecoderTemplateAdapter, webrtc::LibvpxVp9DecoderTemplateAdapter,
//                        webrtc::OpenH264DecoderTemplateAdapter, webrtc::Dav1dDecoderTemplateAdapter>>();
//        media_deps.audio_processing = nullptr;//webrtc::AudioProcessingBuilder().Create();
    }

    void WebRtcServerImpl::InitPeerConnectionFactory() {
        configuration_.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
        configuration_.media_config.video.periodic_alr_bandwidth_probing = true;

        auto ice_server = webrtc::PeerConnectionInterface::IceServer();
        ice_server.uri = "turn:82.157.48.233:3478";
        ice_server.username = "test";
        ice_server.password = "123456";
        configuration_.servers.push_back(ice_server);

        network_thread_ = rtc::Thread::CreateWithSocketServer();
        network_thread_->Start();
        worker_thread_ = rtc::Thread::Create();
        worker_thread_->Start();
        signaling_thread_ = rtc::Thread::Create();
        signaling_thread_->Start();

        webrtc::PeerConnectionFactoryDependencies media_deps;
        media_deps.task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
        CreateSomeMediaDeps(media_deps);

        peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
                network_thread_.get(), worker_thread_.get(), signaling_thread_.get(),
                nullptr,
                webrtc::CreateBuiltinAudioEncoderFactory(),
                webrtc::CreateBuiltinAudioDecoderFactory(),
                std::make_unique<H264EncoderFactory>(),
                webrtc::CreateBuiltinVideoDecoderFactory(),
                nullptr, nullptr);

        if (peer_connection_factory_.get() == nullptr) {
            std::cout << ":" << std::this_thread::get_id() << ":" << "Error on CreateModularPeerConnectionFactory." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "after init ...." << std::endl;
    }

    void WebRtcServerImpl::InitPeerConnection() {
        auto peer_conn_observer = this->GetPeerConnObserver();
        auto result = peer_connection_factory_->CreatePeerConnectionOrError(configuration_,
                                                                            webrtc::PeerConnectionDependencies(peer_conn_observer.get()));
        if (!result.ok()) {
            std::cerr << "create peer connection failed: " << result.error().message() << std::endl;
            return;
        }
        std::cout << "after create peer connection" << std::endl;
        auto peer_conn = result.value();

        if (peer_conn.get() == nullptr) {
            peer_connection_factory_ = nullptr;
            std::cout << ":" << std::this_thread::get_id() << ":" << "Error on CreatePeerConnection." << std::endl;
            exit(EXIT_FAILURE);
        }

        video_track_ = SelfGenVideoTrack::Create();
        auto video_track = peer_connection_factory_->CreateVideoTrack("video_track", video_track_);
        peer_conn->AddTrack(video_track, { "video" });

        this->SetPeerConnection(peer_conn);
        auto options = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions();
        options.offer_to_receive_audio = false;
        options.offer_to_receive_video = true;
        auto create_session_observer = this->GetCreateSessionDescObserver();
        peer_conn->CreateOffer(create_session_observer.get(), options);
    }

    void WebRtcServerImpl::OnImageEncoded(const MsgVideoFrameEncoded& enc_msg) {
        if (!video_track_ || !enc_msg.image_ || !enc_msg.image_->data) {
            return;
        }
        rtc::scoped_refptr<webrtc::EncodedImageBuffer> encodedImageBuffer = webrtc::EncodedImageBuffer::Create((uint8_t*)enc_msg.image_->data->DataAddr(), enc_msg.image_->data->Size());
        rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer = new rtc::RefCountedObject<EncodedVideoFrameBuffer>(enc_msg.frame_width_,enc_msg.frame_height_, encodedImageBuffer);
        auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
        static uint16_t id;
        ++id;
        webrtc::VideoFrame videoFrame = webrtc::VideoFrame::Builder().
                set_video_frame_buffer(buffer).
                set_timestamp_us(ts * rtc::kNumMicrosecsPerMillisec).
                set_id(id).
                build();
        video_track_->AddFrame(videoFrame);
        LOGI("Add frame: {}", id);
    }
}