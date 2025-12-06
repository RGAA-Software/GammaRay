//
// Created by RGAA on 6/08/2025.
//

#ifndef GAMMARAYPREMIUM_PANEL_COMPANION_H
#define GAMMARAYPREMIUM_PANEL_COMPANION_H

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "hw_info/hw_info.h"

namespace tc
{

    // Authorization
    class Authorization {
    public:
        std::string auth_id_;
        std::string appkey_;
    };

    // SpvrSrvConfig
    class SpvrSrvConfig {
    public:
        std::string srv_name_;
        std::string srv_w3c_ip_;
        int srv_spvr_port_ = 0;
        std::string srv_appkey_;
        int srv_relay_port_ = 0;

    public:
        [[nodiscard]] bool IsValid() const {
            return !srv_w3c_ip_.empty() && srv_spvr_port_ > 0 && !srv_appkey_.empty() && srv_relay_port_ > 0;
        }
    };

    // RelaySrvConfig
    class RelaySrvConfig {
    public:
        std::string srv_name_;
        std::string srv_type_;
        std::string srv_w3c_ip_;
        int srv_working_port_ = 0;
        std::string srv_appkey_;

    public:
        [[nodiscard]] bool IsValid() const {
            return !srv_w3c_ip_.empty() && srv_working_port_ > 0 && !srv_appkey_.empty();
        }
    };

    // Spvr Access
    class SpvrAccessInfo {
    public:
        [[nodiscard]] bool IsValid() const {
            return spvr_config_.IsValid();
        }

    public:
        SpvrSrvConfig spvr_config_;
    };

    //
    class PanelCompanion {
    public:
        virtual ~PanelCompanion();

        virtual bool Init() = 0;

        // Timer
        virtual void OnTimer100ms() = 0;
        virtual void OnTimer1S() = 0;
        virtual void OnTimer5S() = 0;

        // Spvr
        virtual void UpdateSpvrServerConfig(const std::string& host, int port) = 0;
        virtual std::shared_ptr<Authorization> RequestAuth() = 0;
        virtual std::shared_ptr<Authorization> GetAuth() = 0;

        // enc
        virtual bool EncQRCode(std::string origin_content, std::vector<uint8_t>& cipher_data) = 0;

        // parse hardware info
        virtual void UpdateCurrentCpuFrequency(float freq) = 0;
        virtual float GetCurrentCpuFrequency() = 0;
        virtual std::shared_ptr<SysInfo> ParseHardwareInfo(const std::string& info) = 0;

        // spvr access
        virtual std::shared_ptr<SpvrAccessInfo> ParseSpvrAccessInfo(const std::string& info) = 0;
    };

}

#endif //GAMMARAYPREMIUM_PANEL_COMPANION_H
