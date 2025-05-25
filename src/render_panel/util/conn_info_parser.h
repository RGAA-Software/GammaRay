//
// Created by RGAA on 20/05/2025.
//

#ifndef GAMMARAY_CONN_INFO_PARSER_H
#define GAMMARAY_CONN_INFO_PARSER_H

#include <memory>
#include <string>
#include <vector>

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
        bool IsValid() const {
            return render_srv_port_ > 0 && !hosts_.empty();
        }

    public:
        std::string device_id_;
        std::string random_pwd_;
        int icon_idx_{0};
        std::vector<GrConnectionHost> hosts_;
        int panel_srv_port_{0};
        int render_srv_port_{0};
    };

    // parse gammaray://xxxx
    class ConnInfoParser {
    public:
        static std::shared_ptr<GrConnectionInfo> Parse(const std::string& info);
    };

}

#endif //GAMMARAY_CONN_INFO_PARSER_H
