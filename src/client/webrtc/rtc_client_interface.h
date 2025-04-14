//
// Created by RGAA on 13/04/2025.
//

#ifndef GAMMARAY_RTC_CLIENT_INTERFACE_H
#define GAMMARAY_RTC_CLIENT_INTERFACE_H

#include <string>
#include <functional>

namespace tc
{

    using OnLocalSdpSetCallback = std::function<void(const std::string&)>;
    using OnLocalIceCallback = std::function<void(const std::string& ice, const std::string& mid, int sdp_mline_index)>;

    class RtcClientInterface {
    public:
        virtual bool Init() {
            return false;
        }

        virtual bool OnRemoteSdp(const std::string& sdp) {
            return false;
        }

        virtual bool OnRemoteIce(const std::string& ice, const std::string& mid, int32_t sdp_mline_index) {
            return false;
        }

        virtual void SetOnLocalSdpSetCallback(OnLocalSdpSetCallback&& cbk) {
            local_sdp_set_cbk_ = cbk;
        }

        virtual void SetOnLocalIceCallback(OnLocalIceCallback&& cbk) {
            local_ice_cbk_ = cbk;
        }

    protected:
        OnLocalSdpSetCallback local_sdp_set_cbk_;
        OnLocalIceCallback local_ice_cbk_;
    };

}

#endif //GAMMARAY_RTC_CLIENT_INTERFACE_H
