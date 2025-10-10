//
// Created by RGAA on 21/09/2025.
//

#include "hw_info_parser.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"
#include "tc_3rdparty/json/json.hpp"
#include "render_panel/companion/panel_companion.h"

using namespace nlohmann;

namespace tc
{

    std::shared_ptr<SysInfo> HWInfoParser::ParseHWInfo(const std::string& input, float current_cpu_freq) {
        try {
            auto value = std::make_shared<SysInfo>();
            auto obj = json::parse(input);
            // CPU
            if (obj.contains("cpu")) {
                auto cpu_obj = obj["cpu"];
                // usage
                value->cpu_.usage_ = cpu_obj["usage"].get<float>();
                // vendor
                value->cpu_.vendor_ = cpu_obj["vendor"].get<std::string>();
                //brand
                value->cpu_.brand_ = StringUtil::Trim(cpu_obj["brand"].get<std::string>());
                // base_frequency
                value->cpu_.base_frequency_ = cpu_obj["base_frequency"].get<float>();
                // current_frequency
                value->cpu_.current_frequency_ = current_cpu_freq;
                // max_frequency
                value->cpu_.max_frequency_ = cpu_obj["max_frequency"].get<float>();
                // cpus
                if (cpu_obj.contains("cpus") && cpu_obj["cpus"].is_array()) {
                    for (const auto& sub : cpu_obj["cpus"]) {
                        value->cpu_.cpus_.push_back(SysSingleCpuInfo {
                            .name_ = sub["name"].get<std::string>(),
                            .usage_ = sub["usage"].get<float>(),
                        });
                    }
                }

            }

            // Memory
            if (obj.contains("mem")) {
                auto mem_obj = obj["mem"];
                // total
                value->mem_.total_ = mem_obj["total"].get<uint64_t>();
                value->mem_.total_gb_ = mem_obj["total_gb"].get<uint64_t>();
                // used
                value->mem_.used_ = mem_obj["used"].get<uint64_t>();
                value->mem_.used_gb_ = mem_obj["used_gb"].get<uint64_t>();
                // available
                value->mem_.available_ = mem_obj["available"].get<uint64_t>();
                value->mem_.available_gb_ = mem_obj["available_gb"].get<uint64_t>();
            }

            // Disks
            if (obj.contains("disks") && obj["disks"].is_array()) {
                for (const auto& disk : obj["disks"]) {
                    value->disks_.push_back(SysDiskInfo {
                        .disk_type_ = disk["disk_type"].get<std::string>(),
                        .mount_on_ = disk["mount_on"].get<std::string>(),
                        .filesystem_ = disk["filesystem"].get<std::string>(),
                        .available_ = disk["available"].get<uint64_t>(),
                        .available_gb_ = disk["available_gb"].get<uint64_t>(),
                        .total_ = disk["total"].get<uint64_t>(),
                        .total_gb_ = disk["total_gb"].get<uint64_t>(),
                    });
                }
            }

            // Network
            if (obj.contains("networks") && obj["networks"].is_array()) {
                for (const auto& network : obj["networks"]) {
                    auto nts = network["ip_networks"];
                    std::vector<SysIpNetwork> ip_networks;
                    for (const auto& nt : nts) {
                        ip_networks.push_back(SysIpNetwork {
                            .addr_ = nt["addr"].get<std::string>(),
                            .prefix_ = nt["prefix"].get<uint8_t>(),
                        });
                    }
                    value->networks_.push_back(SysNetworkInfo {
                        .name_ = network["name"].get<std::string>(),
                        .mac_ = network["mac"].get<std::string>(),
                        .ip_networks_ = ip_networks,
                        .received_data_ = network["received_data"].get<uint64_t>(),
                        .sent_data_ = network["sent_data"].get<uint64_t>(),
                        .max_transmit_speed_ = network["max_transmit_speed"].get<uint64_t>(),
                        .max_receive_speed_ = network["max_receive_speed"].get<uint64_t>(),
                    });
                }
            }

            // OS
            value->os_.sys_name_ = obj["os"]["sys_name"].get<std::string>();
            value->os_.sys_kernel_version_ = obj["os"]["sys_kernel_version"].get<std::string>();
            value->os_.sys_os_version_ = obj["os"]["sys_os_version"].get<std::string>();
            value->os_.sys_os_long_version_ = obj["os"]["sys_os_long_version"].get<std::string>();
            value->os_.sys_host_name_ = obj["os"]["sys_host_name"].get<std::string>();
            value->os_.sys_kernel_ = obj["os"]["sys_kernel"].get<std::string>();

            // Uptime
            value->uptime_ = obj["uptime"].get<std::string>();

            // GPU
            if (obj.contains("gpus") && obj["gpus"].is_array()) {
                auto gpus_obj = obj["gpus"];
                for (const auto& gpu : gpus_obj) {
                    value->gpus_.push_back(SysGpuInfo {
                        .id_ = gpu["id"].get<std::string>(),
                        .brand_ = gpu["brand"].get<std::string>(),
                        .fan_speed_ = gpu["fan_speed"].get<uint32_t>(),
                        .power_limit_ = gpu["power_limit"].get<uint32_t>(),
                        .encoder_utilization_ = gpu["encoder_utilization"].get<uint32_t>(),
                        .gpu_utilization_ = gpu["gpu_utilization"].get<uint32_t>(),
                        .mem_utilization_ = gpu["mem_utilization"].get<uint32_t>(),
                        .temperature_ = gpu["temperature"].get<uint32_t>(),
                        .mem_used_ = gpu["mem_used"].get<uint64_t>(),
                        .mem_used_gb_ = gpu["mem_used_gb"].get<float>(),
                        .mem_total_ = gpu["mem_total"].get<uint64_t>(),
                        .mem_total_gb_ = gpu["mem_total_gb"].get<float>(),
                    });
                }
            }

            auto print_info = to_string(*value.get());
            //LOGI("SysInfo: {}", print_info);

            return value;
        }
        catch(std::exception& e) {
            LOGE("parse json failed: {}", e.what());
            return nullptr;
        }
    }

}