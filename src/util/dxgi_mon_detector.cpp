#include "dxgi_mon_detector.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"

#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")

namespace tc
{

    void DxgiMonitorDetector::DetectAdapters() {
        infos_.clear();

        CComPtr<IDXGIFactory1> pFactory;
        HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **) (&pFactory));
        if (FAILED(hr)) {
            LOGE("CreateDXGIFactory failed.");
            return;
        }
        int max_devices = 16;

        for (int i = 0; i < max_devices; i++) {
            CComPtr<IDXGIAdapter1> tmp_adapter;
            if (pFactory->EnumAdapters1(i, &tmp_adapter) != S_OK) {
                break;
            }

            DxgiMonInfo info;
            DXGI_ADAPTER_DESC adapterDesc;
            tmp_adapter->GetDesc(&adapterDesc);
            info.HighPart = adapterDesc.AdapterLuid.HighPart;
            info.LowPart = adapterDesc.AdapterLuid.LowPart;

            for (int j = 0; j < max_devices; j++) {
                CComPtr<IDXGIOutput> dxgi_output;
                auto r = tmp_adapter->EnumOutputs(j, &dxgi_output);
                if (dxgi_output && SUCCEEDED(r)) {
                    DXGI_OUTPUT_DESC desc;
                    dxgi_output->GetDesc(&desc);

                    info.display_name = StringExt::ToUTF8(desc.DeviceName);
                    info.rect = desc.DesktopCoordinates;
                    info.width = info.rect.right - info.rect.left;
                    info.height = info.rect.bottom - info.rect.top;
                    infos_.push_back(info);
                }
            }
        }
    }

    std::vector<DxgiMonInfo> DxgiMonitorDetector::GetAdapters() {
        return infos_;
    }

    void DxgiMonitorDetector::PrintAdapters() {
        for (auto &info: infos_) {
            LOGI("dxgi monitor, name : {}, resolution : {} x {}, LowPart : {}",
                 info.display_name.c_str(), info.width, info.height, info.LowPart);
        }
    }

    std::string DxgiMonitorDetector::GetNameById(DWORD lowpart) {
        for (auto &info: infos_) {
            if (info.LowPart == lowpart) {
                return info.display_name;
            }
        }
        return "";
    }

    DxgiMonInfo DxgiMonitorDetector::GetMonitorInfoByLowId(DWORD lowpart) {
        DxgiMonInfo empty_info;
        for (auto &info: infos_) {
            if (info.LowPart == lowpart) {
                return info;
            }
        }
        return empty_info;
    }

}