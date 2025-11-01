//
// Created by RGAA on 20/05/2025.
//

#include "conn_info_parser.h"
#include "tc_3rdparty/json/json.hpp"
#include "tc_spvr_client/spvr_stream.h"
#include "tc_common_new/log.h"
#include "tc_common_new/base64.h"

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<GrConnectionInfo> ConnInfoParser::Parse(const std::string& info) {
        std::string prefix = "gammaray://";
        if (!info.starts_with(prefix)) {
            return nullptr;
        }
        auto target_info = info.substr(prefix.size());
        target_info = Base64::Base64Decode(target_info);

        try {
            auto conn_info = std::make_shared<GrConnectionInfo>();
            auto obj = json::parse(target_info);

            conn_info->device_id_ = obj["device_id"].get<std::string>();
            conn_info->random_pwd_ = obj["random_pwd"].get<std::string>();
            conn_info->icon_idx_ = obj["icon_idx"].get<int>();
            auto ips_array = obj["ips"];
            if (ips_array.is_array()) {
                for (const auto& item : ips_array) {
                    GrConnectionInfo::GrConnectionHost host;
                    host.ip_ = item["ip"].get<std::string>();
                    host.type_ = item["type"].get<std::string>();
                    conn_info->hosts_.push_back(host);
                }
            }

            conn_info->panel_srv_port_ = obj["panel_srv_port"].get<int>();
            conn_info->render_srv_port_ = obj["render_srv_port"].get<int>();

            if (!obj["relay_host"].is_null()) {
                conn_info->relay_host_ = obj["relay_host"].get<std::string>();
            }
            if (!obj["relay_port"].is_null()) {
                conn_info->relay_port_ = obj["relay_port"].get<int>();
            }
            if (!obj["relay_appkey"].is_null()) {
                conn_info->relay_appkey_ = obj["relay_appkey"].get<std::string>();
            }

            return conn_info;
        }
        catch(std::exception& e) {
            LOGE("Parse gammaray:// failed: {}, err: {}", target_info, e.what());
            return nullptr;
        }
    }

}