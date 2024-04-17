////
//// Created by hy on 2024/2/1.
////
//
#ifndef WEBRTC_WEBRTC_CLIENT_H
#define WEBRTC_WEBRTC_CLIENT_H

#include "webrtc_helper.h"
#include "observers/video_frame_observer.h"
#include "self_gen_video_track.h"
#include "h264_encoder_factory.h"
#include "app/app_messages.h"

namespace tc
{

    class PeerConnObserverImpl;
    class SetSessionDescObserverImpl;
    class CreateSessionDescObserverImpl;

    class EncodedVideoI420Buffer : public webrtc::I420BufferInterface
    {
    public:
        EncodedVideoI420Buffer(int width, int height, const rtc::scoped_refptr<webrtc::EncodedImageBufferInterface> &encoded_data) : width_(width), height_(height), encoded_data_(encoded_data)
        {
        }
        virtual int width() const { return width_; }
        virtual int height() const { return height_; }
        virtual const uint8_t *DataY() const { return encoded_data_->data(); }
        virtual const uint8_t *DataU() const { return NULL; }
        virtual const uint8_t *DataV() const { return NULL; }
        virtual int StrideY() const { return encoded_data_->size(); }
        virtual int StrideU() const { return 0; }
        virtual int StrideV() const { return 0; }

    private:
        const int width_;
        const int height_;
        rtc::scoped_refptr<webrtc::EncodedImageBufferInterface> encoded_data_;
    };

    class EncodedVideoFrameBuffer : public webrtc::VideoFrameBuffer
    {
    public:
        EncodedVideoFrameBuffer(int width, int height, const rtc::scoped_refptr<webrtc::EncodedImageBufferInterface> &encoded_data)
                : buffer_(new rtc::RefCountedObject<EncodedVideoI420Buffer>(width, height, encoded_data)) {}
        virtual Type type() const { return webrtc::VideoFrameBuffer::Type::kNative; }
        virtual rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() { return webrtc::I420Buffer::Create(width(), height()); }
        virtual int width() const { return buffer_->width(); }
        virtual int height() const { return buffer_->height(); }
        const webrtc::I420BufferInterface *GetI420() const final { return buffer_.get(); }

    private:
        rtc::scoped_refptr<EncodedVideoI420Buffer> buffer_;
    };

    class NotifyFrameFrameBuffer : public webrtc::VideoFrameBuffer
    {
    public:
        NotifyFrameFrameBuffer(int width,int height): mWidth(width),mHeight(height){}
        virtual Type type() const { return webrtc::VideoFrameBuffer::Type::kNative; }
        virtual rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() { return nullptr; }
        virtual int width() const { return mWidth; }
        virtual int height() const { return mHeight; }
        int mWidth;
        int mHeight;
    };

    class WebRtcServerParam {
    public:
        RtcDecodedVideoFrame::RtcFrameType frame_type_;
        std::string remote_ip_;
        int port_;
        OnFrameCallback frame_callback_;
    };

    class WebRtcServerImpl : public std::enable_shared_from_this<WebRtcServerImpl> {
    public:

        static std::shared_ptr<WebRtcServerImpl> Make();

        WebRtcServerImpl();
        void Init(const WebRtcServerParam& param);
        void Exit();

        std::shared_ptr<PeerConnObserverImpl> GetPeerConnObserver();
        std::shared_ptr<VideoFrameObserver> GetVideoFrameObserver();
        rtc::scoped_refptr<SetSessionDescObserverImpl> GetSetSessionDescObserver();
        rtc::scoped_refptr<CreateSessionDescObserverImpl> GetCreateSessionDescObserver();
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> GetPeerConnection();
        void SetPeerConnection(const rtc::scoped_refptr<webrtc::PeerConnectionInterface>& pc);

        void OnImageEncoded(const MsgVideoFrameEncoded& enc_msg);

        // callbacks
        void OnSessionCreated(webrtc::SessionDescriptionInterface *desc);
        void OnIceCandidate(const webrtc::IceCandidateInterface *candidate);
        void OnIceGatheringComplete();
        void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);

    private:
        void RequestRemoteSDP();
        static void CreateSomeMediaDeps(webrtc::PeerConnectionFactoryDependencies& media_deps);
        void InitPeerConnectionFactory();
        void InitPeerConnection();

    private:
        std::string sdp_;
        WebRtcServerParam client_param_;
        std::shared_ptr<PeerConnObserverImpl> peer_conn_observer_ = nullptr;
        std::shared_ptr<VideoFrameObserver> video_frame_observer_ = nullptr;
        rtc::scoped_refptr<SetSessionDescObserverImpl> set_session_desc_observer_ = nullptr;
        rtc::scoped_refptr<CreateSessionDescObserverImpl> create_session_desc_observer_ = nullptr;
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_conn_ = nullptr;
        std::unique_ptr<rtc::Thread> network_thread_;
        std::unique_ptr<rtc::Thread> worker_thread_;
        std::unique_ptr<rtc::Thread> signaling_thread_;
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
        webrtc::PeerConnectionInterface::RTCConfiguration configuration_;
        SelfGenVideoTrack* video_track_ = nullptr;
    };

}

#endif