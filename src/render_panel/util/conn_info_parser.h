//
// Created by RGAA on 20/05/2025.
//

#ifndef GAMMARAY_CONN_INFO_PARSER_H
#define GAMMARAY_CONN_INFO_PARSER_H

#include <memory>
#include <string>
#include <vector>
#include <sstream>

namespace tc
{

    class GrConnectionInfo {
    public:
        class GrConnectionHost {
        public:
            std::string ip_;
            std::string type_;
        };

    public:
        std::string device_id_;
        std::string random_pwd_;
        int icon_idx_{0};
        std::vector<GrConnectionHost> hosts_;
        int panel_srv_port_{0};
        int render_srv_port_{0};
        // relay server
        std::string relay_host_;
        int relay_port_{0};

    public:
        [[nodiscard]] bool IsValid() const {
            return render_srv_port_ > 0 && !hosts_.empty();
        }

        std::string Dump() {
            std::stringstream ss;
            ss << "Connection info: " << std::endl;
            ss << "Device Info: " << device_id_ << ", " << random_pwd_ << ", relay: " << relay_host_ << "," << relay_port_ << std::endl;
            ss << "Device IP:" << std::endl;
            for (const auto& ch : hosts_) {
                ss << ch.ip_ << "" << std::endl;
            }
            return ss.str();
        }
    };

    // parse gammaray://xxxx
    class ConnInfoParser {
    public:
        static std::shared_ptr<GrConnectionInfo> Parse(const std::string& info);
    };

}

#endif //GAMMARAY_CONN_INFO_PARSER_H
