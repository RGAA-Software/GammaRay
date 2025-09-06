#include "d3d11_render_manager.h"
#include <array>
#include "tc_common_new/log.h"
#include "tc_common_new/string_util.h"

using namespace DirectX;

// Below are lists of errors expect from Dxgi API calls when a transition event like mode change, PnpStop, PnpStart
// desktop switch, TDR or session disconnect/reconnect. In all these cases we want the application to clean up the threads that process
// the desktop updates and attempt to recreate them.
// If we get an error that is not on the appropriate list then we exit the application

// These are the errors we expect from general Dxgi API due to a transition
HRESULT SystemTransitionsExpectedErrors[] = {
	DXGI_ERROR_DEVICE_REMOVED,
	DXGI_ERROR_ACCESS_LOST,
	static_cast<HRESULT>(WAIT_ABANDONED),
	S_OK                                    // Terminate list with zero valued HRESULT
};

namespace tc
{

    _Post_satisfies_(return != DUPL_RETURN_SUCCESS)
    DUPL_RETURN ProcessFailure(ComPtr<ID3D11Device> Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ HRESULT* ExpectedErrors = nullptr);

    D3D11RenderManager::D3D11RenderManager() {
    }

    D3D11RenderManager::~D3D11RenderManager() {
        CleanRefs();
    }

    void D3D11RenderManager::WindowResize() {
        m_NeedsResize = true;
    }

    DUPL_RETURN D3D11RenderManager::InitOutput(HWND window, int frame_width, int frame_height, ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> device_context) {
        HRESULT hr;

        // Store window handle
        m_WindowHandle = window;

        m_Device = device;
        m_DeviceContext = device_context;

        // Get DXGI factory
        ComPtr<IDXGIDevice> DxgiDevice = nullptr;
        hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(DxgiDevice.GetAddressOf()));
        if (FAILED(hr)) {
            return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr, nullptr);
        }

        ComPtr<IDXGIAdapter> DxgiAdapter = nullptr;
        hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(DxgiAdapter.GetAddressOf()));
        DxgiDevice.Reset();

        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        hr = DxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(m_Factory.GetAddressOf()));
        DxgiAdapter.Reset();

        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to get parent DXGI Factory", L"Error", hr, SystemTransitionsExpectedErrors);
        }
    //
    //
    //    // Register for occlusion status windows message
    //    hr = m_Factory->RegisterOcclusionStatusWindow(Window, OCCLUSION_STATUS_MSG, &m_OcclusionCookie);
    //    if (FAILED(hr))
    //    {
    //        return ProcessFailure(m_Device, L"Failed to register for occlusion message", L"Error", hr, SystemTransitionsExpectedErrors);
    //    }

        // Get window size
        RECT WindowRect;
        GetClientRect(m_WindowHandle, &WindowRect);
        UINT width = WindowRect.right - WindowRect.left;
        UINT height = WindowRect.bottom - WindowRect.top;

        // Create swapchain for window
        DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
        RtlZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));

        SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        SwapChainDesc.BufferCount = 2;
        SwapChainDesc.Width = width;
        SwapChainDesc.Height = height;
        SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.SampleDesc.Count = 1;
        SwapChainDesc.SampleDesc.Quality = 0;
        hr = m_Factory->CreateSwapChainForHwnd(m_Device.Get(), window, &SwapChainDesc, nullptr, nullptr, m_SwapChain.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create window swapchain", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Disable the ALT-ENTER shortcut for entering full-screen mode
        hr = m_Factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to make window association", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Create shared texture
        DUPL_RETURN Return = CreateTexture(frame_width, frame_height);
        if (Return != DUPL_RETURN_SUCCESS) {
            return Return;
        }

        // Make new render target view
        Return = MakeRTV();
        if (Return != DUPL_RETURN_SUCCESS) {
            return Return;
        }

        // Set view port
        SetViewPort(width, height);
        D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());

        hr = m_Device->CreateSamplerState(&desc, m_SamplerLinear.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create sampler state in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Initialize shaders
        Return = InitShaders();
        if (Return != DUPL_RETURN_SUCCESS) {
            return Return;
        }

        // If window was resized, resize swapchain
        if (m_NeedsResize) {
            DUPL_RETURN Ret = ResizeSwapChain();
            if (Ret != DUPL_RETURN_SUCCESS) {
                return Ret;
            }
            m_NeedsResize = false;
        }

        // Vertices for drawing whole texture
        VERTEX Vertices[NUMVERTICES] = {
            {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
            {XMFLOAT3(-1.0f, 1.0f, 0),  XMFLOAT2(0.0f, 0.0f)},
            {XMFLOAT3(1.0f, -1.0f, 0),  XMFLOAT2(1.0f, 1.0f)},
            {XMFLOAT3(1.0f, -1.0f, 0),  XMFLOAT2(1.0f, 1.0f)},
            {XMFLOAT3(-1.0f, 1.0f, 0),  XMFLOAT2(0.0f, 0.0f)},
            {XMFLOAT3(1.0f, 1.0f, 0),   XMFLOAT2(1.0f, 0.0f)},
        };

        // Rendering NV12 requires two resource views, which represent the luminance and chrominance channels of the YUV formatted texture.
        std::array<ID3D11ShaderResourceView*, 2> const textureViews = {
            m_luminanceView.Get(),
            m_chrominanceView.Get()
        };

        // Bind the NV12 channels to the shader.
        m_DeviceContext->PSSetShaderResources(
            0,
            textureViews.size(),
            textureViews.data()
        );

        // Set resources
        UINT Stride = sizeof(VERTEX);
        UINT Offset = 0;

        D3D11_BUFFER_DESC BufferDesc;
        RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
        BufferDesc.Usage = D3D11_USAGE_DEFAULT;
        BufferDesc.ByteWidth = sizeof(VERTEX) * NUMVERTICES;
        BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BufferDesc.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA InitData;
        RtlZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = Vertices;

        // Create vertex buffer
        hr = m_Device->CreateBuffer(&BufferDesc, &InitData, VertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create vertex buffer when drawing a frame", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        m_DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &Stride, &Offset);
        return Return;
    }

    //
    // Recreate shared texture
    //
    DUPL_RETURN D3D11RenderManager::CreateTexture(int frame_width, int frame_height) {
        HRESULT hr;

        // Get DXGI resources
        ComPtr<IDXGIDevice> DxgiDevice = nullptr;
        hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(DxgiDevice.GetAddressOf()));
        if (FAILED(hr)) {
            return ProcessFailure(nullptr, L"Failed to QI for DXGI Device", L"Error", hr);
        }

        ComPtr<IDXGIAdapter> DxgiAdapter = nullptr;
        hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(DxgiAdapter.GetAddressOf()));
        DxgiDevice.Reset();

        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to get parent DXGI Adapter", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        m_width = frame_width;
        m_height = frame_height;

        DxgiAdapter.Reset();

        D3D11_TEXTURE2D_DESC const texDesc = CD3D11_TEXTURE2D_DESC(
            DXGI_FORMAT_NV12,           // HoloLens PV camera format, common for video sources
            m_width,                    // Width of the video frames
            m_height,                    // Height of the video frames
            1,                          // Number of textures in the array
            1,                          // Number of miplevels in each texture
            D3D11_BIND_SHADER_RESOURCE, // We read from this texture in the shader
            D3D11_USAGE_DEFAULT,
            0
            //		D3D11_USAGE_DYNAMIC,        // Because we'll be copying from CPU memory
            //		D3D11_CPU_ACCESS_WRITE      // We only need to write into the texture
        );


        hr = m_Device->CreateTexture2D(&texDesc, nullptr, m_texture.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create texture", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
        // To access DXGI_FORMAT_NV12 in the shader, we need to map the luminance channel and the chrominance channels
        // into a format that shaders can understand.
        // In the case of NV12, DirectX understands how the texture is laid out, so we can create these
        // shader resource views which represent the two channels of the NV12 texture.
        // Then inside the shader we convert YUV into RGB so we can render.

        // DirectX specifies the view format to be DXGI_FORMAT_R8_UNORM for NV12 luminance channel.
        // Luminance is 8 bits per pixel. DirectX will handle converting 8-bit integers into normalized
        // floats for use in the shader.
        D3D11_SHADER_RESOURCE_VIEW_DESC const luminancePlaneDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
            m_texture.Get(),
            D3D11_SRV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8_UNORM
        );

        hr = m_Device->CreateShaderResourceView(
            m_texture.Get(),
            &luminancePlaneDesc,
            m_luminanceView.GetAddressOf()
        );
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create texture", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // DirectX specifies the view format to be DXGI_FORMAT_R8G8_UNORM for NV12 chrominance channel.
        // Chrominance has 4 bits for U and 4 bits for V per pixel. DirectX will handle converting 4-bit
        // integers into normalized floats for use in the shader.
        D3D11_SHADER_RESOURCE_VIEW_DESC const chrominancePlaneDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(
            m_texture.Get(),
            D3D11_SRV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8G8_UNORM
        );

        hr = m_Device->CreateShaderResourceView(
            m_texture.Get(),
            &chrominancePlaneDesc,
            m_chrominanceView.GetAddressOf()
        );
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create shader resource view", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        return DUPL_RETURN_SUCCESS;
    }

    DUPL_RETURN D3D11RenderManager::ReleaseTexture() {
        if (m_texture) {
            m_texture.Reset();
        }
        if (m_luminanceView) {
            m_luminanceView.Reset();
        }
        if (m_chrominanceView) {
            m_chrominanceView.Reset();
        }
        return DUPL_RETURN_SUCCESS;
    }

    DUPL_RETURN D3D11RenderManager::RecreateTexture(int frame_width, int frame_height) {
        LOGI("Recreate texture with size: {}x{}", frame_width, frame_height);
        ReleaseTexture();
        return CreateTexture(frame_width, frame_height);
    }

    //
    // Present to the application window
    //
    DUPL_RETURN D3D11RenderManager::UpdateApplicationWindow(_Inout_ bool *Occluded) {
        // In a typical desktop duplication application there would be an application running on one system collecting the desktop images
        // and another application running on a different system that receives the desktop images via a network and display the image. This
        // sample contains both these aspects into a single application.
        // This routine is the part of the sample that displays the desktop image onto the display

        HRESULT hr;
        //draw
        DUPL_RETURN Ret = DrawFrame();

        // Present to window if all worked
        if (Ret == DUPL_RETURN_SUCCESS) {
            // Present to window
            hr = m_SwapChain->Present(0, 0);
            if (FAILED(hr)) {
                return ProcessFailure(m_Device, L"Failed to present", L"Error", hr, SystemTransitionsExpectedErrors);
            } else if (hr == DXGI_STATUS_OCCLUDED) {
                *Occluded = true;
            }
        } else {
            LOGE("Draw failed: {}", Ret);
        }

        return Ret;
    }

    //
    // Draw frame into backbuffer
    //
    DUPL_RETURN D3D11RenderManager::DrawFrame() {
        // If window was resized, resize swapchain
        if (m_NeedsResize) {
            {
                std::lock_guard<std::mutex> guard(resize_mtx_);
                DUPL_RETURN Ret = ResizeSwapChain();
                if (Ret != DUPL_RETURN_SUCCESS) {
                    return Ret;
                }
            }
            m_NeedsResize = false;
        }

        D3DCOLORVALUE m_BackColor{0.0f, 0.135f, 0.481f, 1.0f};
        m_DeviceContext->ClearRenderTargetView(m_RTV.Get(), reinterpret_cast<const float *>(&m_BackColor));

        FLOAT blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
        m_DeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        m_DeviceContext->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);

        // Rendering NV12 requires two resource views, which represent the luminance and chrominance channels of the YUV formatted texture.
        std::array<ID3D11ShaderResourceView*, 2> const textureViews = {
            m_luminanceView.Get(),
            m_chrominanceView.Get()
        };
        // Bind the NV12 channels to the shader.
        m_DeviceContext->PSSetShaderResources(
            0,
            textureViews.size(),
            textureViews.data()
        );

        // Set resources
        UINT Stride = sizeof(VERTEX);
        UINT Offset = 0;
        m_DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &Stride, &Offset);

        // Draw textured quad onto render target
        m_DeviceContext->Draw(NUMVERTICES, 0);

        // 解除资源绑定（避免资源泄漏警告）
        ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
        m_DeviceContext->PSSetShaderResources(0, 2, nullSRVs);

        return DUPL_RETURN_SUCCESS;
    }

    //
    // Initialize shaders for drawing to screen
    //
    DUPL_RETURN D3D11RenderManager::InitShaders() {
        HRESULT hr;

        UINT Size = ARRAYSIZE(g_VS);
        hr = m_Device->CreateVertexShader(g_VS, Size, nullptr, m_VertexShader.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create vertex shader in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 2> Layout = {{
             {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
             {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        }};

        hr = m_Device->CreateInputLayout(Layout.data(), Layout.size(), g_VS, Size, m_InputLayout.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create input layout in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }
        m_DeviceContext->IASetInputLayout(m_InputLayout.Get());

        Size = ARRAYSIZE(g_PS);
        hr = m_Device->CreatePixelShader(g_PS, Size, nullptr, m_PixelShader.GetAddressOf());
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create pixel shader in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        m_DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
        m_DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
        m_DeviceContext->PSSetSamplers(0, 1, m_SamplerLinear.GetAddressOf());
        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return DUPL_RETURN_SUCCESS;
    }

    //
    // Reset render target view
    //
    DUPL_RETURN D3D11RenderManager::MakeRTV() {
        // Get backbuffer
        ComPtr<ID3D11Texture2D> BackBuffer = nullptr;
        HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(BackBuffer.GetAddressOf()));
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to get backbuffer for making render target view in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Create a render target view
        hr = m_Device->CreateRenderTargetView(BackBuffer.Get(), nullptr, m_RTV.GetAddressOf());
        BackBuffer.Reset();
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to create render target view in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Set new render target
        m_DeviceContext->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr);
        return DUPL_RETURN_SUCCESS;
    }

    //
    // Set new viewport
    //
    void D3D11RenderManager::SetViewPort(UINT Width, UINT Height) {
        D3D11_VIEWPORT VP;
        VP.Width = static_cast<FLOAT>(Width);
        VP.Height = static_cast<FLOAT>(Height);
        VP.MinDepth = 0.0f;
        VP.MaxDepth = 1.0f;
        VP.TopLeftX = 0;
        VP.TopLeftY = 0;
        m_DeviceContext->RSSetViewports(1, &VP);
    }

    //
    // Resize swapchain
    //
    DUPL_RETURN D3D11RenderManager::ResizeSwapChain() {
        if (m_RTV) {
            m_RTV.Reset();
        }

        RECT WindowRect;
        GetClientRect(m_WindowHandle, &WindowRect);
        UINT Width = WindowRect.right - WindowRect.left;
        UINT Height = WindowRect.bottom - WindowRect.top;

        LOGI("Will resize swap chain: {}x{}", Width, Height);
        // Resize swapchain
        DXGI_SWAP_CHAIN_DESC SwapChainDesc;
        m_SwapChain->GetDesc(&SwapChainDesc);
        HRESULT hr = m_SwapChain->ResizeBuffers(SwapChainDesc.BufferCount, Width, Height,
                                                SwapChainDesc.BufferDesc.Format, SwapChainDesc.Flags);
        if (FAILED(hr)) {
            return ProcessFailure(m_Device, L"Failed to resize swapchain buffers in OutputManager", L"Error", hr, SystemTransitionsExpectedErrors);
        }

        // Make new render target view
        DUPL_RETURN Ret = MakeRTV();
        if (Ret != DUPL_RETURN_SUCCESS) {
            return Ret;
        }

        // Set new viewport
        SetViewPort(Width, Height);
        return Ret;
    }

    //
    // Releases all references
    //
    void D3D11RenderManager::CleanRefs() {
        if (m_VertexShader) {
            m_VertexShader.Reset();
        }

        if (m_PixelShader) {
            m_PixelShader.Reset();
        }

        if (m_InputLayout) {
            m_InputLayout.Reset();
        }

        if (m_RTV) {
            m_RTV.Reset();
        }

        if (m_SamplerLinear) {
            m_SamplerLinear.Reset();
        }

        if (m_BlendState) {
            m_BlendState.Reset();
        }

        if (m_DeviceContext) {
            m_DeviceContext.Reset();
        }

        if (m_Device) {
            m_Device.Reset();
        }

        if (m_SwapChain) {
            m_SwapChain.Reset();
        }

        if (m_luminanceView) {
            m_luminanceView.Reset();
        }

        if (m_chrominanceView) {
            m_chrominanceView.Reset();
        }

        if (m_Factory) {
            if (m_OcclusionCookie) {
                m_Factory->UnregisterOcclusionStatus(m_OcclusionCookie);
                m_OcclusionCookie = 0;
            }
            m_Factory.Reset();
        }
    }


    DUPL_RETURN ProcessFailure(ComPtr<ID3D11Device> Device, _In_ LPCWSTR Str, _In_ LPCWSTR Title, HRESULT hr, _In_opt_z_ HRESULT *ExpectedErrors) {
        HRESULT TranslatedHr;
        // On an error check if the DX device is lost
        if (Device) {
            HRESULT DeviceRemovedReason = Device->GetDeviceRemovedReason();
            switch (DeviceRemovedReason) {
                case DXGI_ERROR_DEVICE_REMOVED:
                case DXGI_ERROR_DEVICE_RESET:
                case static_cast<HRESULT>(E_OUTOFMEMORY): {
                    // Our device has been stopped due to an external event on the GPU so map them all to
                    // device removed and continue processing the condition
                    TranslatedHr = DXGI_ERROR_DEVICE_REMOVED;
                    break;
                }

                case S_OK: {
                    // Device is not removed so use original error
                    TranslatedHr = hr;
                    break;
                }

                default: {
                    // Device is removed but not a error we want to remap
                    TranslatedHr = DeviceRemovedReason;
                }
            }
        } else {
            TranslatedHr = hr;
        }

        LOGE("Error: 0x{:x}, message: {}", hr, tc::StringUtil::ToUTF8(Str));

        // Check if this error was expected or not
        if (ExpectedErrors) {
            HRESULT *CurrentResult = ExpectedErrors;
            while (*CurrentResult != S_OK) {
                if (*(CurrentResult++) == TranslatedHr) {
                    return DUPL_RETURN_ERROR_EXPECTED;
                }
            }
        }

        // Error was not expected so display the message box
        //DisplayMsg(Str, Title, TranslatedHr);
        LOGE("Unexpected error: 0x{:x}", TranslatedHr);
        return DUPL_RETURN_ERROR_UNEXPECTED;
    }
}