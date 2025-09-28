//
// Created by RGAA on 28/09/2025.
//

#ifndef GAMMARAYPREMIUM_GR_NET_PLUGIN_TYPE_H
#define GAMMARAYPREMIUM_GR_NET_PLUGIN_TYPE_H

namespace tc
{

    enum class NetPluginType {
        kWebSocket,
        kUdpKcp,
        kWebRtcDirect,
        kWebRtc
    };

    enum class NetChannelType {
        kMedia,
        kFileTransfer,
    };

    class NetMessageAck {
    public:
        uint64_t send_time_ = 0;
        uint64_t resp_time_ = 0;
        NetChannelType ch_type_;
        int msg_type_ = 0; // MessageType
    };
}

#endif //GAMMARAYPREMIUM_GR_NET_PLUGIN_TYPE_H
