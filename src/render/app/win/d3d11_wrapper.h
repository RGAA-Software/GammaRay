//
// Created by RGAA on 1/05/2025.
//

#ifndef GAMMARAY_D3D11_WRAPPER_H
#define GAMMARAY_D3D11_WRAPPER_H

#include <d3d11.h>
#include <memory>

namespace tc
{

    class D3D11DeviceWrapper {
    public:
        void Release() const {
            if (d3d11_device_context_) {
                d3d11_device_context_->ClearState();
                d3d11_device_context_->Flush();
                d3d11_device_context_->Release();
            }
            if (d3d11_device_) {
                d3d11_device_->Release();
            }
        }

    public:
        uint64_t adapter_uid_ = 0;
        ComPtr<ID3D11Device> d3d11_device_ = nullptr;
        ComPtr<ID3D11DeviceContext> d3d11_device_context_ = nullptr;
    };

}

#endif //GAMMARAY_D3D11_WRAPPER_H
