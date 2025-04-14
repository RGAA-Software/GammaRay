//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>
#include <memory>
#include "gr_plugin_interface.h"
#include "gr_net_plugin.h"
#include "tc_capture_new/capture_message.h"
#include "tc_common_new/time_util.h"

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
        kPluginClipboardEvent,
        kPluginRelayPausedEvent,
        kPluginRelayResumeEvent,
        kPluginRtcAnswerSdpEvent,
        kPluginRtcIceEvent,
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
        std::string message_;
        int64_t socket_fd_ = 0;
        NetPluginType nt_plugin_type_;
    };

    // GrClientConnectedEvent
    class GrPluginClientConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientConnectedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginClientConnectedEvent;
        }

    };

    // GrClientDisConnectedEvent
    class GrPluginClientDisConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientDisConnectedEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginClientDisConnectedEvent;
        }

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

    // Clipboard message
    class GrPluginClipboardEvent : public GrPluginBaseEvent {
    public:
        GrPluginClipboardEvent() : GrPluginBaseEvent() {
            event_type_ = GrPluginEventType::kPluginClipboardEvent;
        }
    public:
        int type_;
        std::string msg_;
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
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
