//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>
#include <memory>
#include "gr_plugin_interface.h"

namespace tc
{

    class Data;
    class Image;

    enum class GrPluginEventType {
        kPluginUnknownType,
        kPluginKeyboardEvent,
        kPluginMouseEvent,
        kPluginGamePadEvent,
        kPluginClientConnectedEvent,
        kPluginClientDisConnectedEvent,
        kPluginInsertIdrEvent,
        kPluginEncodedVideoFrameEvent,
    };

    class GrPluginBaseEvent {
    public:
        virtual ~GrPluginBaseEvent() = default;
    public:
        std::string plugin_name_;
        GrPluginEventType plugin_type_{GrPluginEventType::kPluginUnknownType};
        std::any extra_;
    };

    // GrKeyboardEvent
    class GrPluginKeyboardEvent : public GrPluginBaseEvent {
    public:

    };

    // GrMouseEvent
    class GrPluginMouseEvent : public GrPluginBaseEvent {
    public:

    };

    // GrGamePadEvent
    class GrPluginGamePadEvent : public GrPluginBaseEvent {
    public:

    };

    // GrClientConnectedEvent
    class GrPluginClientConnectedEvent : public GrPluginBaseEvent {
    public:

    };

    // GrClientDisConnectedEvent
    class GrPluginClientDisConnectedEvent : public GrPluginBaseEvent {
    public:

    };

    // GrPluginInsertIdrEvent
    class GrPluginInsertIdrEvent : public GrPluginBaseEvent {
    public:

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
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
