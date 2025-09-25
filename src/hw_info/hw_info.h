//
// Created by RGAA on 20/09/2025.
//

#ifndef GAMMARAYPREMIUM_PANEL_HARDWARE_INFO_H
#define GAMMARAYPREMIUM_PANEL_HARDWARE_INFO_H

#include <string>
#include <vector>
#include <sstream>

namespace tc
{

    class SysSingleCpuInfo {
    public:
        std::string name_;
        float usage_ = 0.0f;
    };

    class SysCpuInfo {
    public:
        float usage_ = 0.0f;
        std::string vendor_;
        std::string brand_;
        float base_frequency_ = 0.0f;
        float current_frequency_ = 0.0f;
        float max_frequency_ = 0.0f;
        std::vector<SysSingleCpuInfo> cpus_;
    };

    class SysMemInfo {
    public:
        uint64_t total_ = 0;
        uint64_t total_gb_ = 0;
        uint64_t used_ = 0;
        uint64_t used_gb_ = 0;
        uint64_t available_ = 0;
        uint64_t available_gb_ = 0;
    };

    class SysDiskInfo {
    public:
        std::string disk_type_;
        std::string mount_on_;
        std::string filesystem_;
        uint64_t available_ = 0;
        uint64_t available_gb_ = 0;
        uint64_t total_ = 0;
        uint64_t total_gb_ = 0;
    };

    class SysIpNetwork {
    public:
        std::string addr_;
        uint8_t prefix_ = 0;
    };

    class SysNetworkInfo {
    public:
        std::string name_;
        std::string mac_;
        std::vector<SysIpNetwork> ip_networks_;
        uint64_t received_data_ = 0;
        uint64_t sent_data_ = 0;
        uint64_t max_transmit_speed_ = 0;
        uint64_t max_receive_speed_ = 0;
    };

    class SysUserInfo {
    public:

    };

    class SysOsInfo {
    public:
        std::string sys_name_;
        std::string sys_kernel_version_;
        std::string sys_os_version_;
        std::string sys_os_long_version_;
        std::string sys_host_name_;
        std::string sys_kernel_;
    };

    class SysComponentInfo {
    public:
        float temperature_ = 0.0f;
        float max_ = 0.0f;
        float critical_ = 0.0f;
        std::string label_;
    };

    class SysGpuInfo {
    public:
        std::string id_;
        std::string brand_;
        uint32_t fan_speed_ = 0;
        uint32_t power_limit_ = 0;
        uint32_t encoder_utilization_ = 0;
        uint32_t gpu_utilization_ = 0;
        uint32_t mem_utilization_ = 0;
        uint32_t temperature_ = 0;
        uint64_t mem_free_ = 0;
        float mem_free_gb_ = 0;
        uint64_t mem_used_ = 0;
        float mem_used_gb_ = 0;
        uint64_t mem_total_ = 0;
        float  mem_total_gb_ = 0;
    };

    class SysInfo {
    public:
        SysCpuInfo cpu_;
        SysMemInfo mem_;
        std::vector<SysDiskInfo> disks_;
        std::vector<SysNetworkInfo> networks_;
        SysOsInfo os_;
        std::vector<SysComponentInfo> components_;
        std::string uptime_;
        std::vector<SysGpuInfo> gpus_;
    };

    // PRINT
    // 辅助函数：将字节转换为GB字符串
    static std::string bytes_to_gb_str(uint64_t bytes) {
        double gb = static_cast<double>(bytes) / (1024 * 1024 * 1024);
        std::stringstream ss;
        ss.precision(2);
        ss << std::fixed << gb << " GB";
        return ss.str();
    }

    // 为每个子类编写打印函数
    static std::string to_string(const SysSingleCpuInfo& cpu) {
        std::stringstream ss;
        ss << "SysSingleCpuInfo{name='" << cpu.name_
           << "', usage=" << cpu.usage_ << "}";
        return ss.str();
    }

    static std::string to_string(const SysCpuInfo& cpu) {
        std::stringstream ss;
        ss << "SysCpuInfo{usage=" << cpu.usage_
           << ", vendor='" << cpu.vendor_
           << "', brand='" << cpu.brand_
           << "', base_frequency=" << cpu.base_frequency_
           << ", current_frequency=" << cpu.current_frequency_
           << ", max_frequency=" << cpu.max_frequency_
           << ", cpus=[";

        for (size_t i = 0; i < cpu.cpus_.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << to_string(cpu.cpus_[i]);
        }
        ss << "]}";
        return ss.str();
    }

    static std::string to_string(const SysMemInfo& mem) {
        std::stringstream ss;
        ss << "SysMemInfo{total=" << mem.total_ << " (" << bytes_to_gb_str(mem.total_)
           << "), used=" << mem.used_ << " (" << bytes_to_gb_str(mem.used_)
           << "), available=" << mem.available_ << " (" << bytes_to_gb_str(mem.available_)
           << ")}";
        return ss.str();
    }

    static std::string to_string(const SysDiskInfo& disk) {
        std::stringstream ss;
        ss << "SysDiskInfo{disk_type='" << disk.disk_type_
           << "', mount_on='" << disk.mount_on_
           << "', filesystem='" << disk.filesystem_
           << "', available=" << disk.available_ << " (" << bytes_to_gb_str(disk.available_)
           << "), total=" << disk.total_ << " (" << bytes_to_gb_str(disk.total_)
           << ")}";
        return ss.str();
    }

    static std::string to_string(const SysIpNetwork& ip_net) {
        std::stringstream ss;
        ss << "SysIpNetwork{addr='" << ip_net.addr_
           << "', prefix=" << static_cast<int>(ip_net.prefix_)
           << "}";
        return ss.str();
    }

    static std::string to_string(const SysNetworkInfo& net) {
        std::stringstream ss;
        ss << "SysNetworkInfo{name='" << net.name_
           << "', mac='" << net.mac_
           << "', received_data=" << net.received_data_
           << ", sent_data=" << net.sent_data_
           << ", max_transmit_speed =" << net.max_transmit_speed_
           << ", max_receive_speed=" << net.max_receive_speed_
           << ", ip_networks=[";

        for (size_t i = 0; i < net.ip_networks_.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << to_string(net.ip_networks_[i]);
        }
        ss << "]}";
        return ss.str();
    }

    static std::string to_string(const SysOsInfo& os) {
        std::stringstream ss;
        ss << "SysOsInfo{sys_name='" << os.sys_name_
           << "', sys_kernel_version='" << os.sys_kernel_version_
           << "', sys_os_version='" << os.sys_os_version_
           << "', sys_os_long_version='" << os.sys_os_long_version_
           << "', sys_host_name='" << os.sys_host_name_
           << "', sys_kernel='" << os.sys_kernel_
           << "'}";
        return ss.str();
    }

    static std::string to_string(const SysComponentInfo& comp) {
        std::stringstream ss;
        ss << "SysComponentInfo{label='" << comp.label_
           << "', temperature=" << comp.temperature_
           << ", max=" << comp.max_
           << ", critical=" << comp.critical_
           << "}";
        return ss.str();
    }

    static std::string to_string(const SysGpuInfo& gpu) {
        std::stringstream ss;
        ss << "SysGpuInfo{brand='" << gpu.brand_
           << "', id=" << gpu.id_
           << "', fan_speed=" << gpu.fan_speed_
           << ", power_limit=" << gpu.power_limit_
           << ", encoder_utilization=" << gpu.encoder_utilization_
           << ", gpu_utilization=" << gpu.gpu_utilization_
           << ", mem_utilization=" << gpu.mem_utilization_
           << ", temperature=" << gpu.temperature_
           << ", mem_free=" << gpu.mem_free_ << " (" << bytes_to_gb_str(gpu.mem_free_)
           << "), mem_used=" << gpu.mem_used_ << " (" << bytes_to_gb_str(gpu.mem_used_)
           << "), mem_total=" << gpu.mem_total_ << " (" << bytes_to_gb_str(gpu.mem_total_)
           << ")}";
        return ss.str();
    }

    // 主要的打印函数
    static std::string to_string(const SysInfo& sys_info) {
        std::stringstream ss;

        ss << "SysInfo{\n";
        ss << "  cpu: " << to_string(sys_info.cpu_) << ",\n";
        ss << "  mem: " << to_string(sys_info.mem_) << ",\n";

        ss << "  disks: [\n";
        for (size_t i = 0; i < sys_info.disks_.size(); ++i) {
            ss << "    " << to_string(sys_info.disks_[i]);
            if (i < sys_info.disks_.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ],\n";

        ss << "  networks: [\n";
        for (size_t i = 0; i < sys_info.networks_.size(); ++i) {
            ss << "    " << to_string(sys_info.networks_[i]);
            if (i < sys_info.networks_.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ],\n";

        ss << "  os: " << to_string(sys_info.os_) << ",\n";

        ss << "  components: [\n";
        for (size_t i = 0; i < sys_info.components_.size(); ++i) {
            ss << "    " << to_string(sys_info.components_[i]);
            if (i < sys_info.components_.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ],\n";

        ss << "  uptime: '" << sys_info.uptime_ << "',\n";

        ss << "  gpus: [\n";
        for (size_t i = 0; i < sys_info.gpus_.size(); ++i) {
            ss << "    " << to_string(sys_info.gpus_[i]);
            if (i < sys_info.gpus_.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << "  ]\n";

        ss << "}";

        return ss.str();
    }


}

#endif //GAMMARAYPREMIUM_PANEL_HARDWARE_INFO_H
