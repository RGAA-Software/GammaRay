#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <d3d11.h>
#include <Dxgi1_6.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include <comdef.h>
#include <Wbemidl.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <codecvt>
#include "tc_common_new/log.h"

namespace tc
{

    class DxgiMonInfo {
    public:
        DWORD LowPart = 0;
        LONG HighPart = 0;
        std::string display_name{};
        RECT rect{};
        int width = 0;
        int height = 0;
        bool primary = false;

    public:
        [[nodiscard]] bool IsValid() const {
            return width > 0 && height > 0;
        }

        void Dump() {
            LOGI("Monitor Info: {}, primary: {}, LowPart: {}, ({},{}), {}x{}",
                 display_name, primary, LowPart, rect.left, rect.top, width, height);
        }

    };

    class DxgiMonitorDetector {
    public:

        static DxgiMonitorDetector *Instance() {
            static DxgiMonitorDetector detector;
            return &detector;
        }

        void DetectAdapters();
        std::vector<DxgiMonInfo> GetAdapters();
        void PrintAdapters();
        std::string GetNameById(DWORD lowpart);
        DxgiMonInfo GetMonitorInfoByLowId(DWORD lowpart);

    private:
        std::vector<DxgiMonInfo> infos_;

    };

}