//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_RTC_CLIENT_INTERFACE_H
#define GAMMARAY_RTC_CLIENT_INTERFACE_H

namespace tc
{

    class RtcClientInterface {
    public:
        virtual bool Init() {
            return false;
        }
    };

}

#endif //GAMMARAY_RTC_CLIENT_INTERFACE_H
