#ifndef D3D11_RENDER_MANAGER_H_
#define D3D11_RENDER_MANAGER_H_

#include <cstdio>
#include <mutex>
#include "d3d11_common_types.h"
#include "warning.h"
#include "tc_common_new/win32/d3d11_wrapper.h"

namespace tc
{
    class D3D11RenderManager {
    public:
        D3D11RenderManager();
        ~D3D11RenderManager();

        DUPL_RETURN InitOutput(HWND window, int frame_width, int frame_height, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
        DUPL_RETURN UpdateApplicationWindow(_Inout_ bool *Occluded);

        void CleanRefs();
        void WindowResize();
        DUPL_RETURN RecreateTexture(int frame_width, int frame_height);
        ComPtr<ID3D11Texture2D> GetTexture() { return m_texture; }

    private:
        // Methods
        DUPL_RETURN MakeRTV();
        void SetViewPort(UINT Width, UINT Height);
        DUPL_RETURN InitShaders();
        DUPL_RETURN CreateTexture(int frame_width, int frame_height);
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
        ComPtr<ID3D11PixelShader> m_PixelShader = nullptr;
        ComPtr<ID3D11InputLayout> m_InputLayout = nullptr;
        HWND m_WindowHandle = nullptr;
        bool m_NeedsResize = false;
        DWORD m_OcclusionCookie = 0;
        std::mutex resize_mtx_;
    };

}
#endif
