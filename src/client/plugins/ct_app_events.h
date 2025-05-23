//
// Created by RGAA on 23/05/2025.
//

#ifndef GAMMARAY_CT_APP_EVENTS_H
#define GAMMARAY_CT_APP_EVENTS_H

#include <memory>
#include <string>

// from exe ---> plugins

namespace tc
{

    enum class AppEventType {
        kUnknownAppEvent,

        // test begin
        kTestEvent
        // test end
    };

    class ClientAppBaseEvent {
    public:
        AppEventType evt_type_ = AppEventType::kUnknownAppEvent;
    };

    // test begin
    class ClientAppTestEvent : public ClientAppBaseEvent {
    public:
        ClientAppTestEvent() : ClientAppBaseEvent() {
            evt_type_ = AppEventType::kTestEvent;
        }
    };
    // test end
}

#endif //GAMMARAY_CT_APP_EVENTS_H
