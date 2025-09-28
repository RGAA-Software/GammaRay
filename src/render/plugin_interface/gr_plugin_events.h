//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>
#include <memory>
#include "gr_net_plugin.h"
#include "gr_plugin_interface.h"
#include "tc_common_new/image.h"
#include "tc_common_new/time_util.h"
#include "tc_capture_new/capture_message.h"

namespace tc
{

    class Data;
    class Image;

    enum class GrPluginEventType {
        kPluginUnknownType,
        kPluginNetClientEvent,
        kPluginClientConnectedEvent,
        kPluginClientDisConnectedEvent,
        kPluginInsertIdrEvent,
        kPluginEncodedVideoFrameEvent,
        kPluginCapturingMonitorInfoEvent,
        kPluginCapturedVideoFrameEvent,
        kPluginCursorEvent,
        kPluginRawVideoFrameEvent,
        kPluginRawAudioFrameEvent,
        kPluginSplitRawAudioFrameEvent,
        kPluginEncodedAudioFrameEvent,
        kPluginRelayPausedEvent,
        kPluginRelayResumeEvent,
        kPluginRtcAnswerSdpEvent,
        kPluginRtcIceEvent,
        kPluginRtcReportEvent,
        kPluginFileTransferBegin,
        kPluginFileTransferEnd,
        kPluginDataSent,
        kPluginRemoteClipboardResp,
        kPluginPanelStreamMessage,
        kPluginConfigEncoder,
    };

    class GrPluginBaseEvent {
    public:
        GrPluginBaseEvent() {
            created_timestamp_ = TimeUtil::GetCurrentTimestamp();
        }
        virtual ~GrPluginBaseEvent() = default;
    public:
        std::string plugin_name_;
        GrPluginEventType event_type_{GrPluginEventType::kPluginUnknownType};
        std::any extra_;
        uint64_t created_timestamp_ = 0;
    };

    // kPluginNetClientEvent
    class GrPluginNetClientEvent : public GrPluginBaseEvent {
    public:
        GrPluginNetClientEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginNetClientEvent;
        }
    public:
        bool is_proto_ = true;
        std::shared_ptr<Data> message_;
        int64_t socket_fd_ = 0;
        NetPluginType nt_plugin_type_;
        NetChannelType nt_channel_type_;
        GrNetPlugin* from_plugin_ = nullptr;
    };

    // GrClientConnectedEvent
    class GrPluginClientConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientConnectedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginClientConnectedEvent;
        }
    public:
        std::string conn_id_;
        std::string stream_id_;
        std::string conn_type_;
        std::string visitor_device_id_;
        int64_t begin_timestamp_ = 0;
    };

    // GrClientDisConnectedEvent
    class GrPluginClientDisConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientDisConnectedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginClientDisConnectedEvent;
        }
    public:
        std::string conn_id_;
        std::string stream_id_;
        std::string visitor_device_id_;
        int64_t end_timestamp_ = 0;
        int64_t duration_ = 0;
    };

    // GrPluginInsertIdrEvent
    class GrPluginInsertIdrEvent : public GrPluginBaseEvent {
    public:
        GrPluginInsertIdrEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginInsertIdrEvent;
        }
    };

    // GrPluginEncodedVideoFrameEvent
    class GrPluginEncodedVideoFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginEncodedVideoFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginEncodedVideoFrameEvent;
        }
    public:
        GrPluginEncodedVideoType type_;
        std::shared_ptr<Data> data_ = nullptr;
        uint32_t frame_width_ = 0;
        uint32_t frame_height_ = 0;
        bool key_frame_ = false;
        uint64_t frame_index_ = 0;
        RawImageType frame_format_ = RawImageType::kI420;
    };

    //
    class GrPluginCapturedVideoFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginCapturedVideoFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginCapturedVideoFrameEvent;
        }
    public:
        CaptureVideoFrame frame_;
    };

    //
    class GrPluginCapturingMonitorInfoEvent : public GrPluginBaseEvent {
    public:
        GrPluginCapturingMonitorInfoEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginCapturingMonitorInfoEvent;
        }
    public:

    };

    //
    class GrPluginCursorEvent : public GrPluginBaseEvent {
    public:
        GrPluginCursorEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginCursorEvent;
        }
    public:
        CaptureCursorBitmap cursor_info_;
    };

    // Raw video frame from plugins
    class GrPluginRawVideoFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginRawVideoFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRawVideoFrameEvent;
        }
    public:
        std::shared_ptr<Image> image_ = nullptr;
        uint64_t frame_index_ = 0;
        uint64_t frame_format_ = 0;
    };

    // Raw audio frame from plugins
    class GrPluginRawAudioFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginRawAudioFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRawAudioFrameEvent;
        }
    public:
        // left/right/left/right...
        std::shared_ptr<Data> full_data_ = nullptr;
        //
        int sample_rate_ = 0;
        int channels_ = 0;
        int bits_ = 0;
    };

    // Raw audio frame
    class GrPluginSplitRawAudioFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginSplitRawAudioFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginSplitRawAudioFrameEvent;
        }
    public:
        // left/left/left/...
        std::shared_ptr<Data> left_ch_data_ = nullptr;
        // right/right/right...
        std::shared_ptr<Data> right_ch_data_ = nullptr;
        //
        int sample_rate_ = 0;
        int channels_ = 0;
        int bits_ = 0;
    };

    // Encode audio frame
    class GrPluginEncodedAudioFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginEncodedAudioFrameEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginEncodedAudioFrameEvent;
        }
    public:
        int sample_rate_ = 0;
        int channels_ = 0;
        int bits_ = 0;
        int frame_size_ = 0;
        std::shared_ptr<Data> data_ = nullptr;
    };

    // relay paused
    class GrPluginRelayPausedEvent : public GrPluginBaseEvent {
    public:
        GrPluginRelayPausedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRelayPausedEvent;
        }
    };

    // relay resumed
    class GrPluginRelayResumedEvent : public GrPluginBaseEvent {
    public:
        GrPluginRelayResumedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRelayResumeEvent;
        }
    };

    // rtc answer
    class GrPluginRtcAnswerSdpEvent : public GrPluginBaseEvent {
    public:
        GrPluginRtcAnswerSdpEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRtcAnswerSdpEvent;
        }

    public:
        std::string stream_id_;
        std::string sdp_;
    };

    // rtc ice
    class GrPluginRtcIceEvent : public GrPluginBaseEvent {
    public:
        GrPluginRtcIceEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRtcIceEvent;
        }
    public:
        std::string stream_id_;
        std::string ice_;
        std::string mid_;
        int sdp_mline_index_{0};
    };

    // rtc report event
    class GrPluginRtcReportEvent : public GrPluginBaseEvent {
    public:
        GrPluginRtcReportEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRtcReportEvent;
        }
    public:
        std::string evt_name_;
        std::string msg_;
        std::string data_channel_name_;
    };

    // file transfer begin
    class GrPluginFileTransferBegin : public GrPluginBaseEvent {
    public:
        GrPluginFileTransferBegin() {
            event_type_ = GrPluginEventType::kPluginFileTransferBegin;
        }
    public:
        std::string the_file_id_;
        int64_t begin_timestamp_ = 0;
        std::string visitor_device_id_;
        std::string direction_;
        std::string file_detail_;
    };

    // file transfer end
    class GrPluginFileTransferEnd : public GrPluginBaseEvent {
    public:
        GrPluginFileTransferEnd() {
            event_type_ = GrPluginEventType::kPluginFileTransferEnd;
        }
    public:
        bool success_ = false;
        std::string the_file_id_;
        int64_t end_timestamp_ = 0;
        int64_t duration_ = 0;
    };

    // data sent size
    class GrPluginDataSent : public GrPluginBaseEvent {
    public:
        GrPluginDataSent() {
            event_type_ = GrPluginEventType::kPluginDataSent;
        }
    public:
        int size_ = 0;
    };

    // remote clipboard resp
    class GrPluginRemoteClipboardResp : public GrPluginBaseEvent {
    public:
        GrPluginRemoteClipboardResp() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginRemoteClipboardResp;
        }
    public:
        // text / file
        int content_type_ {0};
        // text content
        std::string remote_info_;
    };

    // panel stream message
    // request from remote panel
    class GrPluginPanelStreamMessage : public GrPluginBaseEvent {
    public:
        GrPluginPanelStreamMessage() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginPanelStreamMessage;
        }
    public:
        std::shared_ptr<Data> body_ = nullptr;
    };

    // config encoder
    class GrPluginConfigEncoder : public GrPluginBaseEvent {
    public:
        GrPluginConfigEncoder() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginConfigEncoder;
        }
    public:
        std::string mon_name_;
        uint32_t bps_ = 0;
        uint32_t fps_ = 0;
    };
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
