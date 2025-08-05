//
// Created by RGAA on 2024/3/17.
//

#include "dx_address_loader.h"
#include "hook_capture/capture_message.h"
#include "tc_common_new/process_util.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"

#include <vector>
#include <string>

namespace tc
{

    std::shared_ptr<AppSharedMessage> DxAddressLoader::LoadDxAddress() {
        auto cap_message = std::make_shared<AppSharedMessage>();
        auto output = ProcessUtil::StartProcessAndOutput("tc_graphics_helper.exe", {});
        if (output.empty()) {
            LOGE("Failed to run ");
            return nullptr;
        }

        auto fn_get_address_number = [](std::vector<std::string>& parsed_line) -> uint64_t {
            auto hex_str = parsed_line.at(parsed_line.size() - 1);
            auto address = 0;
            try {
                address = std::stoi(hex_str, nullptr, 16);
            } catch (...) {
                LOGE("Convert to integer failed: {}", hex_str);
            }
            return address;
        };

        bool is_d3d9 = false;
        bool is_dxgi = false;
        for (const auto& line : output) {
            LOGI("{}", line);
            if (line.starts_with("[d3d9]")) {
                is_d3d9 = true;
                is_dxgi = false;
                continue;
            }
            if (line.starts_with("[dxgi]")) {
                is_d3d9 = false;
                is_dxgi = true;
                continue;
            }

            if (!is_dxgi && !is_d3d9) {
                continue;
            }

            std::vector<std::string> parsed_line;
            StringUtil::Split(line, parsed_line, "=");
            auto address = fn_get_address_number(parsed_line);

            if (is_d3d9) {
                if (line.starts_with("present=")) {
                    cap_message->d3d9_present = address;
                }
                else if (line.starts_with("present_ex=")) {
                    cap_message->d3d9_present_ex = address;
                }
                else if (line.starts_with("present_swap=")) {
                    cap_message->d3d9_present_swap = address;
                }
                else if (line.starts_with("d3d9_clsoff=")) {
                    cap_message->d3d9_d3d9_clsoff = address;
                }
                else if (line.starts_with("is_d3d9ex_clsoff=")) {
                    cap_message->d3d9_is_d3d9ex_clsoff = address;
                }
            }
            if (is_dxgi) {
                if (line.starts_with("present=")) {
                    cap_message->dxgi_present = address;
                }
                else if (line.starts_with("present1=")) {
                    cap_message->dxgi_present1 = address;
                }
                else if (line.starts_with("resize=")) {
                    cap_message->dxgi_resize = address;
                }
                else if (line.starts_with("release=")) {
                    cap_message->dxgi_release = address;
                }
            }
        }
        return cap_message;
    }

}