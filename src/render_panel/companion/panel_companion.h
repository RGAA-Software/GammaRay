//
// Created by RGAA on 6/08/2025.
//

#ifndef GAMMARAYPREMIUM_PANEL_COMPANION_H
#define GAMMARAYPREMIUM_PANEL_COMPANION_H

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "panel_hardware_info.h"

namespace tc
{

    // Authorization
    class Authorization {
    public:
        std::string auth_id_;
        std::string appkey_;
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
        virtual std::shared_ptr<SysInfo> ParseHardwareInfo(const std::string& info) = 0;
    };

}

#endif //GAMMARAYPREMIUM_PANEL_COMPANION_H
