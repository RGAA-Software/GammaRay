// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef _OUTPUTMANAGER_H_
#define _OUTPUTMANAGER_H_

#include <cstdio>
#include <mutex>
#include "d3d11_common_types.h"
#include "warning.h"
#include "tc_common_new/win32/d3d11_wrapper.h"

namespace tc
{
    //
    // Handles the task of drawing into a window.
    // Has the functionality to draw the mouse given a mouse shape buffer and position
    //
    class D3D11RenderManager {
    public:
        D3D11RenderManager();
        ~D3D11RenderManager();

        DUPL_RETURN InitOutput(HWND Window, int frame_width, int frame_height, ComPtr<ID3D11Device> device,  ComPtr<ID3D11DeviceContext> context);
        DUPL_RETURN UpdateApplicationWindow(_Inout_ bool *Occluded);

        void CleanRefs();
        void WindowResize();
        DUPL_RETURN RecreateTexture(int frame_width, int frame_height);

        // Vars
        ComPtr<ID3D11Device> m_Device;
        ComPtr<ID3D11DeviceContext> m_DeviceContext;
        IDXGISwapChain1 *m_SwapChain;

        ID3D11Texture2D *m_texture;
        ID3D11ShaderResourceView *m_luminanceView;
        ID3D11ShaderResourceView *m_chrominanceView;
        uint32_t m_width;
        uint32_t m_height;
        ID3D11Buffer *VertexBuffer = nullptr;

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
        // Vars
        IDXGIFactory2 *m_Factory;
        ID3D11RenderTargetView *m_RTV;
        ID3D11SamplerState *m_SamplerLinear;
        ID3D11BlendState *m_BlendState;
        ID3D11VertexShader *m_VertexShader;
        ID3D11PixelShader *m_PixelShader;
        ID3D11InputLayout *m_InputLayout;
        HWND m_WindowHandle;
        bool m_NeedsResize;
        DWORD m_OcclusionCookie;
        std::mutex resize_mtx_;
    };

}
#endif
