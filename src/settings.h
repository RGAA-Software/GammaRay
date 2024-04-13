//
// Created by hy on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_SETTINGS_H
#define TC_SERVER_STEAM_SETTINGS_H

namespace tc
{

    class Settings {
    public:

        static Settings* Instance() {
            static Settings st;
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

#endif //TC_SERVER_STEAM_SETTINGS_H
