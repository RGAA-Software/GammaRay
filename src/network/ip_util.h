//
// Created by RGAA on 2023/8/16.
//

#ifndef SAILFISH_SERVER_IPUTIL_H
#define SAILFISH_SERVER_IPUTIL_H

#include <map>
#include <string>

namespace tc
{

    enum class IPNetworkType {
        kWired,
        kWireless,
    };

    class IPUtil {
    public:

        static std::map<std::string, IPNetworkType> ScanIPs();

    };

}

#endif //SAILFISH_SERVER_IPUTIL_H
