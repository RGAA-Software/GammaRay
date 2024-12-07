//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>
#include <memory>
#include "gr_plugin_interface.h"
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
    };

    class GrPluginBaseEvent {
    public:
        virtual ~GrPluginBaseEvent() = default;
    public:
        std::string plugin_name_;
        GrPluginEventType plugin_type_{GrPluginEventType::kPluginUnknownType};
        std::any extra_;
    };

    // kPluginNetClientEvent
    class GrPluginNetClientEvent : public GrPluginBaseEvent {
    public:
        GrPluginNetClientEvent() {
            plugin_type_ = GrPluginEventType::kPluginNetClientEvent;
        }
    public:
        bool is_proto_;
        std::string message_;
    };

    // GrClientConnectedEvent
    class GrPluginClientConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientConnectedEvent() {
            plugin_type_ = GrPluginEventType::kPluginClientConnectedEvent;
        }

    };

    // GrClientDisConnectedEvent
    class GrPluginClientDisConnectedEvent : public GrPluginBaseEvent {
    public:
        GrPluginClientDisConnectedEvent() {
            plugin_type_ = GrPluginEventType::kPluginClientDisConnectedEvent;
        }

    };

    // GrPluginInsertIdrEvent
    class GrPluginInsertIdrEvent : public GrPluginBaseEvent {
    public:
        GrPluginInsertIdrEvent() {
            plugin_type_ = GrPluginEventType::kPluginInsertIdrEvent;
        }
    };

    // GrPluginEncodedVideoFrameEvent
    class GrPluginEncodedVideoFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginEncodedVideoFrameEvent() {
            plugin_type_ = GrPluginEventType::kPluginEncodedVideoFrameEvent;
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
        GrPluginCapturedVideoFrameEvent() {
            plugin_type_ = GrPluginEventType::kPluginCapturedVideoFrameEvent;
        }
    public:
        CaptureVideoFrame frame_;
    };

    //
    class GrPluginCapturingMonitorInfoEvent : public GrPluginBaseEvent {
    public:
        GrPluginCapturingMonitorInfoEvent() {
            plugin_type_ = GrPluginEventType::kPluginCapturingMonitorInfoEvent;
        }
    public:

    };

    //
    class GrPluginCursorEvent : public GrPluginBaseEvent {
    public:
        GrPluginCursorEvent() {
            plugin_type_ = GrPluginEventType::kPluginCursorEvent;
        }
    public:
        CaptureCursorBitmap cursor_info_;
    };

    // Raw video frame from plugins
    class GrPluginRawVideoFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginRawVideoFrameEvent() {
            plugin_type_ = GrPluginEventType::kPluginRawVideoFrameEvent;
        }
    public:
        std::shared_ptr<Image> image_ = nullptr;
        uint64_t frame_index_ = 0;
    };

    // Raw audio frame from plugins
    class GrPluginRawAudioFrameEvent : public GrPluginBaseEvent {
    public:
        GrPluginRawAudioFrameEvent() {
            plugin_type_ = GrPluginEventType::kPluginRawAudioFrameEvent;
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
        GrPluginSplitRawAudioFrameEvent() {
            plugin_type_ = GrPluginEventType::kPluginSplitRawAudioFrameEvent;
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
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
