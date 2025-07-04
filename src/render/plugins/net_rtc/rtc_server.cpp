//
// Created by hy on 2024/4/25.
//

#include "rtc_server.h"
#include "peer_callback.h"
#include "desktop_capture.h"
#include "desktop_capture_source.h"
#include "rtc_plugin.h"
#include "plugin_interface/gr_plugin_events.h"
#include "rtc_data_channel.h"

using namespace webrtc;

namespace tc
{

    std::shared_ptr<RtcServer> RtcServer::Make(RtcPlugin* plugin) {
        return std::make_shared<RtcServer>(plugin);
    }

    RtcServer::RtcServer(RtcPlugin* plugin) {
        plugin_ = plugin;
    }

    RtcPlugin* RtcServer::GetPlugin() {
        return plugin_;
    }

    bool RtcServer::Start(const std::string& stream_id, const std::string& offer_sdp) {
        this->stream_id_ = stream_id;
        this->offer_sdp_ = offer_sdp;
        webrtc::field_trial::InitFieldTrialsFromString("");
        rtc::LogMessage::LogToDebug(rtc::LS_ERROR);
        rtc::InitializeSSL();

        set_remote_offer_sdp_callback_ = SetSessCallback::Make(shared_from_this());
        set_local_answer_sdp_callback_ = SetSessCallback::Make(shared_from_this());
        create_answer_callback_ = CreateSessCallback::Make(shared_from_this());
        peer_callback_ = PeerCallback::Make(shared_from_this());

        // set remote offer sdp
        set_remote_offer_sdp_callback_->SetSdpSuccessCallback([=]() {
            LOGI("Set remote sdp success");
        });

        set_remote_offer_sdp_callback_->SetSdpFailedCallback([=](const std::string& m) {
            LOGE("Set remote sdp failed: {}", m);
        });

        // set local answer sdp
        set_local_answer_sdp_callback_->SetSdpSuccessCallback([=]() {
            LOGI("Set local answer sdp success.");
        });

        set_local_answer_sdp_callback_->SetSdpFailedCallback([=, this](const std::string& m) {
            LOGI("Set local answer sdp failed:{}", m);
        });

        // create answer sdp callback
        create_answer_callback_->SetOnCreateSdpSuccessCallback([=, this](webrtc::SessionDescriptionInterface* desc) {
            LOGI("Create answer sdp success, will set local sdp.");
            peer_conn_->SetLocalDescription(this->set_local_answer_sdp_callback_.get(), desc);
            std::string sdp;
            desc->ToString(&sdp);
            this->sdp_ = sdp;
            // send to remote
            this->SendSdpToRemote(sdp);
        });

        create_answer_callback_->SetOnCreateSdpFailedCallback([=, this](const std::string& m) {
            LOGE("Create answer sdp failed: {}", m);
        });

        // peer connection
        peer_callback_->SetOnIceCallback([=, this](const std::string& ice, const std::string& mid, int sdp_mline_index) {
            LOGI("ICE: {}", ice);
            this->SendIceToRemote(ice, mid, sdp_mline_index);
        });

        peer_callback_->SetOnDataChannelCallback([=, this](const std::string& name, rtc::scoped_refptr<webrtc::DataChannelInterface> ch) {
            if (name == "media_data_channel") {
                media_data_channel_ = std::make_shared<RtcDataChannel>(name, shared_from_this(), ch);

                // data callback
                media_data_channel_->SetOnDataCallback([=, this](const std::string& data) {
                    auto payload_msg = std::string(data.data(), data.size());
                    plugin_->OnClientEventCame(true, 0, NetPluginType::kWebRtc, payload_msg);
                });
            }
            else if (name == "ft_data_channel") {
                ft_data_channel_ = std::make_shared<RtcDataChannel>(name, shared_from_this(), ch);

                // data callback
                ft_data_channel_->SetOnDataCallback([=, this](const std::string& data) {
                    auto payload_msg = std::string(data.data(), data.size());
                    plugin_->OnClientEventCame(true, 0, NetPluginType::kWebRtc, payload_msg);
                });
            }
        });

        // network state
        peer_callback_->SetOnIceConnectedCallback([=, this]() {

        });

        peer_callback_->SetOnIceDisConnectedCallback([=, this]() {
            if (!media_data_channel_) {return;}
            auto event = std::make_shared<GrPluginClientDisConnectedEvent>();
            event->stream_id_ = media_data_channel_->the_conn_id_;
            event->end_timestamp_ = (int64_t) TimeUtil::GetCurrentTimestamp();
            event->duration_ =   event->end_timestamp_ - media_data_channel_->created_timestamp_;
            this->plugin_->CallbackEvent(event);
        });

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
            stun.uri = "stun:39.91.109.105:60498";
            configuration_.servers.push_back(stun);
        }
        if (0) {
            auto turn = webrtc::PeerConnectionInterface::IceServer();
            turn.tls_cert_policy = webrtc::PeerConnectionInterface::TlsCertPolicy::kTlsCertPolicyInsecureNoCheck;
            turn.uri = "turn:syxmsg.xyz:3478";
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
            return;
        }
        LOGI("CreatePeerConnectionFactory success.");
    }

    void RtcServer::CreatePeerConnection() {
        configuration_.port_allocator_config.min_port = 60430;
        configuration_.port_allocator_config.max_port = 60490;
        auto result = peer_conn_factory_->
                CreatePeerConnectionOrError(configuration_, webrtc::PeerConnectionDependencies(peer_callback_.get()));
        if (!result.ok()) {
            std::cerr << "create peer connection failed: " << result.error().message() << std::endl;
            return;
        }
        auto peer_conn = result.value();

        if (peer_conn.get() == nullptr) {
            peer_conn_factory_ = nullptr;
            std::cout << ":" << std::this_thread::get_id() << ":" << "Error on CreatePeerConnection." << std::endl;
            exit(EXIT_FAILURE);
        }
        this->peer_conn_ = peer_conn;

        // set remote sdp
        LOGI("Will set remote offer sdp.");
        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", offer_sdp_, &error));
        peer_conn_->SetRemoteDescription(this->set_remote_offer_sdp_callback_.get(), session_description);
        if (!error.line.empty()) {
            LOGE("OnOfferSdpCallback, SetRemoteDescription error: {}, {}", error.line, error.description);
            return;
        }

        LOGI("Will create answer sdp.");
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        options.offer_to_receive_audio = true;
        options.offer_to_receive_video = true;
        peer_conn_->CreateAnswer(this->create_answer_callback_.get(), options);

    }

    void RtcServer::OnRemoteIce(const std::string& ice, const std::string& mid, int sdp_mline_index) {
        LOGI("OnRemoteIce: {}", ice);
        webrtc::SdpParseError error;
        std::unique_ptr<webrtc::IceCandidateInterface> candidate(webrtc::CreateIceCandidate(mid, sdp_mline_index, ice, &error));
        if (!error.line.empty()) {
            LOGE("Create IceCandidate failed: {} - {}", error.line, error.description);
            return;
        }
        peer_conn_->AddIceCandidate(std::move(candidate), [](webrtc::RTCError error) {
            if (error.ok()) {
                LOGI("AddIceCandidate success.");
            } else {
                LOGE("AddIceCandidate failed: {}", error.message());
            }
        });
    }

    void RtcServer::SendSdpToRemote(const std::string& sdp) {
        auto event = std::make_shared<GrPluginRtcAnswerSdpEvent>();
        event->stream_id_ = stream_id_;
        event->sdp_ = sdp;
        plugin_->CallbackEvent(event);
    }

    void RtcServer::SendIceToRemote(const std::string& ice, const std::string& mid, int sdp_mline_index) {
        auto event = std::make_shared<GrPluginRtcIceEvent>();
        event->stream_id_ = stream_id_;
        event->ice_ = ice;
        event->mid_ = mid;
        event->sdp_mline_index_ = sdp_mline_index;
        plugin_->CallbackEvent(event);
    }

    void RtcServer::PostProtoMessage(const std::string &msg, bool run_through) {
        if (network_thread_ && media_data_channel_ && !exit_) {
            network_thread_->PostTask([=, this]() {
                media_data_channel_->SendData(msg);
            });
        }
    }

    bool RtcServer::PostTargetStreamProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) {
        if (network_thread_ && media_data_channel_ && !exit_) {
            network_thread_->PostTask([=, this]() {
                media_data_channel_->SendData(msg);
            });
        }
        return true;
    }

    bool RtcServer::PostTargetFileTransferProtoMessage(const std::string &stream_id, const std::string &msg, bool run_through) {
        if (ft_data_channel_ && !exit_) {
            ft_data_channel_->SendData(msg);
        }
        return true;
    }

    bool RtcServer::IsDataChannelConnected() {
        return !exit_ && media_data_channel_ && media_data_channel_->IsConnected();
    }

    uint32_t RtcServer::GetMediaPendingMessages() {
        return !exit_ && media_data_channel_ ? media_data_channel_->GetPendingDataCount() : 0;
    }

    uint32_t RtcServer::GetFtPendingMessages() {
        return !exit_ && ft_data_channel_ ? ft_data_channel_->GetPendingDataCount() : 0;
    }

    bool RtcServer::HasEnoughBufferForQueuingMediaMessages() {
        return !exit_ && media_data_channel_ && media_data_channel_->HasEnoughBufferForQueuingMessages();
    }

    bool RtcServer::HasEnoughBufferForQueuingFtMessages() {
        return !exit_ && ft_data_channel_ && ft_data_channel_->HasEnoughBufferForQueuingMessages();
    }

    void RtcServer::On100msTimeout() {
        if (ft_data_channel_ && !exit_) {
            ft_data_channel_->On100msTimeout();
        }
    }

    void RtcServer::Exit() {
        exit_ = true;
        if (media_data_channel_) {
            media_data_channel_->Close();
        }
        if (ft_data_channel_) {
            ft_data_channel_->Close();
        }
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
