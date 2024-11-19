//
// Created by RGAA on 19/11/2024.
//

#include "video_frame_carrier.h"
#include "tc_common_new/log.h"
#include "tc_common_new/string_ext.h"

namespace tc
{

    VideoFrameCarrier::VideoFrameCarrier(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
    }

    bool VideoFrameCarrier::D3D11Texture2DLockMutex(ComPtr<ID3D11Texture2D> texture2d) {
        HRESULT hres;
        ComPtr<IDXGIKeyedMutex> key_mutex;
        hres = texture2d.As<IDXGIKeyedMutex>(&key_mutex);
        if (FAILED(hres)) {
            printf("D3D11Texture2DReleaseMutex IDXGIKeyedMutex. error\n");
            return false;
        }
        hres = key_mutex->AcquireSync(0, INFINITE);
        if (FAILED(hres)) {
            printf("D3D11Texture2DReleaseMutex AcquireSync failed.\n");
            return false;
        }
        return true;
    }

    bool VideoFrameCarrier::D3D11Texture2DReleaseMutex(ComPtr<ID3D11Texture2D> texture2d) {
        HRESULT hres;
        ComPtr<IDXGIKeyedMutex> key_mutex;
        hres = texture2d.As<IDXGIKeyedMutex>(&key_mutex);
        if (FAILED(hres)) {
            printf("D3D11Texture2DReleaseMutex IDXGIKeyedMutex. error\n");
            return false;
        }
        hres = key_mutex->ReleaseSync(0);
        if (FAILED(hres)) {
            printf("D3D11Texture2DReleaseMutex ReleaseSync failed.\n");
            return false;
        }
        return true;
    }

    bool VideoFrameCarrier::CopyID3D11Texture2D(ComPtr<ID3D11Texture2D> shared_texture) {
        if (!D3D11Texture2DLockMutex(shared_texture)) {
            LOGE("D3D11Texture2DLockMutex error");
            return false;
        }
        std::shared_ptr<void> auto_release_texture2D_mutex((void *) nullptr, [=, this](void *temp) {
            D3D11Texture2DReleaseMutex(shared_texture);
        });

        HRESULT hres;
        D3D11_TEXTURE2D_DESC desc;
        shared_texture->GetDesc(&desc);

        ComPtr<ID3D11Device> curDevice;
        shared_texture->GetDevice(&curDevice);

        if (texture2d_) {
            ComPtr<ID3D11Device> sharedTextureDevice;
            texture2d_->GetDevice(&sharedTextureDevice);
            if (sharedTextureDevice != curDevice) {
                texture2d_ = nullptr;
            }
            if (texture2d_) {
                D3D11_TEXTURE2D_DESC sharedTextureDesc;
                texture2d_->GetDesc(&sharedTextureDesc);
                if (desc.Width != sharedTextureDesc.Width ||
                    desc.Height != sharedTextureDesc.Height ||
                    desc.Format != sharedTextureDesc.Format) {
                    texture2d_ = nullptr;
                }
            }
        }

        if (!texture2d_) {
            D3D11_TEXTURE2D_DESC createDesc;
            ZeroMemory(&createDesc, sizeof(createDesc));
            createDesc.Format = desc.Format;
            createDesc.Width = desc.Width;
            createDesc.Height = desc.Height;
            createDesc.MipLevels = 1;
            createDesc.ArraySize = 1;
            createDesc.SampleDesc.Count = 1;
            //createDesc.Usage = D3D11_USAGE_DEFAULT;
            //createDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            createDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            createDesc.Usage = D3D11_USAGE_STAGING;
            hres = curDevice->CreateTexture2D(&createDesc, NULL, texture2d_.GetAddressOf());
            if (FAILED(hres)) {
                LOGE("desktop capture create texture failed with:{}", StringExt::GetErrorStr(hres).c_str());
                return false;
            }
        }
        ComPtr<ID3D11DeviceContext> ctx;
        curDevice->GetImmediateContext(&ctx);
        ctx->CopyResource(texture2d_.Get(), shared_texture.Get());

        return true;
    }

    ComPtr<ID3D11Texture2D> VideoFrameCarrier::OpenSharedTexture(HANDLE handle) {
        ComPtr<ID3D11Texture2D> sharedTexture;
        HRESULT hres;
        hres = d3d11_device_->OpenSharedResource(handle, IID_PPV_ARGS(sharedTexture.GetAddressOf()));
        if (FAILED(hres)) {
            LOGE("OpenSharedResource failed: {}", hres);
            return nullptr;
        }
        return sharedTexture;
    }
    
}