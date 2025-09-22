//
// Created by RGAA on 21/09/2025.
//

#ifndef GAMMARAYPREMIUM_HW_INFO_PARSER_H
#define GAMMARAYPREMIUM_HW_INFO_PARSER_H

#include <memory>
#include <string>

namespace tc
{

    class SysInfo;

    class HWInfoParser {
    public:
        static std::shared_ptr<SysInfo> ParseHWInfo(const std::string& input, float current_cpu_freq);
    };

}

#endif //GAMMARAYPREMIUM_HW_INFO_PARSER_H
