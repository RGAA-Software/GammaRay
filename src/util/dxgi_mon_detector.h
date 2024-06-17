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

    public:
        [[nodiscard]] bool IsValid() const {
            return width > 0 && height > 0;
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