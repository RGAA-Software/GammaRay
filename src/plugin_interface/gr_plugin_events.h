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
        kPluginNetClientEvent,
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
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
