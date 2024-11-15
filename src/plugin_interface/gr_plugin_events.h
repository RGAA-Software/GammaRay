//
// Created by hy on 15/11/2024.
//

#ifndef GAMMARAY_GR_PLUGIN_EVENTS_H
#define GAMMARAY_GR_PLUGIN_EVENTS_H

#include <string>

namespace tc
{

    enum class GrPluginEventType {

    };

    class GrPluginBaseEvent {
    public:
        std::string plugin_name_;
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
}

#endif //GAMMARAY_GR_PLUGIN_EVENTS_H
