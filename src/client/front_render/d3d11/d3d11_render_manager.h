#ifndef D3D11_RENDER_MANAGER_H_
#define D3D11_RENDER_MANAGER_H_

#include <cstdio>
#include <mutex>
#include <QImage>
#include "warning.h"
#include "d3d11_common_types.h"
#include "tc_common_new/win32/d3d11_wrapper.h"
#include "tc_client_sdk_new/gl/raw_image.h"

namespace tc
{
    class D3D11RenderManager {
    public:
        D3D11RenderManager();
        ~D3D11RenderManager();

        DUPL_RETURN InitOutput(HWND window, RawImageFormat raw_format, int frame_width, int frame_height, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
        DUPL_RETURN UpdateApplicationWindow(_Inout_ bool *Occluded);

        void CleanRefs();
        void WindowResize();
        DUPL_RETURN RecreateTexture(RawImageFormat raw_format, int frame_width, int frame_height);
        ComPtr<ID3D11Texture2D> GetTexture() { return m_texture; }
        ComPtr<ID3D11Device> GetDevice() { return m_Device; }
        ComPtr<ID3D11DeviceContext> GetDeviceContext() { return m_DeviceContext; }
        ComPtr<ID3D11Texture2D> GetYPlane() { return texture_plane_[0]; }
        ComPtr<ID3D11Texture2D> GetUPlane() { return texture_plane_[1]; }
        ComPtr<ID3D11Texture2D> GetVPlane() { return texture_plane_[2]; }
        QImage SaveBackBufferToImage();

        int GetFrameWidth();
        int GetFrameHeight();

    private:
        // Methods
        DUPL_RETURN MakeRTV();
        void SetViewPort(UINT Width, UINT Height);
        DUPL_RETURN InitShaders();
        DUPL_RETURN CreateTexture(RawImageFormat raw_format, int frame_width, int frame_height);
        DUPL_RETURN ReleaseTexture();

        DUPL_RETURN DrawFrame();
        DUPL_RETURN ResizeSwapChain();

    private:
        ComPtr<ID3D11Device> m_Device = nullptr;
        ComPtr<ID3D11DeviceContext> m_DeviceContext = nullptr;
        ComPtr<IDXGISwapChain1> m_SwapChain = nullptr;

        ComPtr<ID3D11Texture2D> m_texture = nullptr;
        ComPtr<ID3D11ShaderResourceView> m_luminanceView = nullptr;
        ComPtr<ID3D11ShaderResourceView> m_chrominanceView = nullptr;
        ComPtr<ID3D11Buffer> VertexBuffer = nullptr;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        ComPtr<IDXGIFactory2> m_Factory = nullptr;
        ComPtr<ID3D11RenderTargetView> m_RTV = nullptr;
        ComPtr<ID3D11SamplerState> m_SamplerLinear = nullptr;
        ComPtr<ID3D11BlendState> m_BlendState = nullptr;
        ComPtr<ID3D11VertexShader> m_VertexShader = nullptr;
        ComPtr<ID3D11PixelShader> m_PixelShaderNv12 = nullptr;
        ComPtr<ID3D11PixelShader> m_PixelShader420p = nullptr;
        ComPtr<ID3D11InputLayout> m_InputLayout = nullptr;
        HWND m_WindowHandle = nullptr;
        bool m_NeedsResize = false;
        DWORD m_OcclusionCookie = 0;
        std::mutex resize_mtx_;
        RawImageFormat raw_image_format_;
        ComPtr<ID3D11Texture2D> texture_plane_[3];
        ComPtr<ID3D11ShaderResourceView> shader_plane_[3];

        uint32_t swapchain_width_ = 0;
        uint32_t swapchain_height_ = 0;
    };

}
#endif
