//
// Created by RGAA on 24/03/2025.
//

#ifndef GAMMARAY_CT_STREAM_ITEM_NET_TYPE_H
#define GAMMARAY_CT_STREAM_ITEM_NET_TYPE_H

#include <string>

namespace tc
{

    static std::string kStreamItemNtTypeWebSocket = "websocket";
    static std::string kStreamItemNtTypeUdpKcp = "udp_kcp";
    static std::string kStreamItemNtTypeRelay = "relay";
    static std::string kStreamItemNtTypeWebRTCDirect = "webrtc_direct";
    static std::string kStreamItemNtTypeWebRTC = "webrtc";

    static std::string kStreamItemConnTypeDirect = "direct";
    static std::string kStreamItemConnTypeSignaling = "signaling";

}

#endif //GAMMARAY_CT_STREAM_ITEM_NET_TYPE_H
