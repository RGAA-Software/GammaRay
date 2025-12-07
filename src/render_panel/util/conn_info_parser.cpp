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
        std::string prefix = "link://";
        if (!info.starts_with(prefix)) {
            return nullptr;
        }
        auto target_info = info.substr(prefix.size());
        target_info = Base64::Base64Decode(target_info);

        try {
            LOGI("Origin ConnInfo: {}", target_info);
            auto conn_info = std::make_shared<GrConnectionInfo>();
            auto obj = json::parse(target_info);

            // device_id
            conn_info->device_id_ = obj["did"].get<std::string>();
            // device name
            if (!obj["dn"].is_null()) {
                conn_info->device_name_ = obj["dn"].get<std::string>();
            }
            // random password
            conn_info->random_pwd_ = obj["rpwd"].get<std::string>();
            // icon index
            conn_info->icon_idx_ = obj["iidx"].get<int>();
            // ips
            auto ips_array = obj["ips"];
            if (ips_array.is_array()) {
                for (const auto& item : ips_array) {
                    GrConnectionInfo::GrConnectionHost host;
                    host.ip_ = item["ip"].get<std::string>();
                    //host.type_ = item["type"].get<std::string>();
                    conn_info->hosts_.push_back(host);
                }
            }

            // panel server port
            conn_info->panel_srv_port_ = obj["ppt"].get<int>();
            // render server port
            conn_info->render_srv_port_ = obj["rdpt"].get<int>();

            // relay host
            if (!obj["rlst"].is_null()) {
                conn_info->relay_host_ = obj["rlst"].get<std::string>();
            }

            // relay port
            if (!obj["rlpt"].is_null()) {
                conn_info->relay_port_ = obj["rlpt"].get<int>();
            }

            // relay appkey
            if (!obj["rlak"].is_null()) {
                conn_info->relay_appkey_ = obj["rlak"].get<std::string>();
            }

            return conn_info;
        }
        catch(std::exception& e) {
            LOGE("Parse link:// failed: {}, err: {}", target_info, e.what());
            return nullptr;
        }
    }

}