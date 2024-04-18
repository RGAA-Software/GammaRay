//
// Created by hy on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GR_SETTINGS_H
#define TC_SERVER_STEAM_GR_SETTINGS_H

namespace tc
{

    class GrSettings {
    public:

        static GrSettings* Instance() {
            static GrSettings st;
            return &st;
        }

        void Load();

    public:

        int http_server_port_{0};
        int ws_server_port_{0};
        int udp_server_port_{0};
        int stream_server_port_{0};

    };

}

#endif //TC_SERVER_STEAM_GR_SETTINGS_H
