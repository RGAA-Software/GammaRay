//
// Created by RGAA on 2023/8/16.
//

#ifndef SAILFISH_SERVER_IPUTIL_H
#define SAILFISH_SERVER_IPUTIL_H

#include <map>
#include <string>
#include <vector>

namespace tc
{

    enum class IPNetworkType {
        kWired,
        kWireless,
    };

    class EthernetInfo {
    public:
        std::string human_readable_name_;
        std::string ip_addr_;
        IPNetworkType nt_type_;
    };

    class IPUtil {
    public:

        static std::vector<EthernetInfo> ScanIPs();

    };

}

#endif //SAILFISH_SERVER_IPUTIL_H
