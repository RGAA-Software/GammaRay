#include "frame_render.h"
#include "Utilities.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include <wincodec.h>

using namespace DirectX;

namespace tc
{

    std::shared_ptr<FrameRender> FrameRender::Make(ID3D11Device* device, ID3D11DeviceContext* context) {
        return std::make_shared<FrameRender>(device, context);
    }

    FrameRender::FrameRender(ID3D11Device* device, ID3D11DeviceContext* context) :
        m_Device(device),
        m_DeviceContext(context),
        m_SamplerLinear(nullptr),
        m_BlendState(nullptr),
        m_VertexShader(nullptr),
        m_PixelShader(nullptr),
        m_InputLayout(nullptr),
        m_TargetTexture(nullptr),
        m_RTV(nullptr),
        m_SrcTexture(nullptr),
        m_SrcSrv(nullptr)
    {}

    FrameRender::~FrameRender() {
        CleanRefs();
    }

    HRESULT FrameRender::InitializeDesc(_In_ SIZE size, _Out_ D3D11_TEXTURE2D_DESC* pTargetDesc, int format) {
        // Create shared texture for the target view
        RtlZeroMemory(pTargetDesc, sizeof(D3D11_TEXTURE2D_DESC));
        pTargetDesc->Width = size.cx;
        pTargetDesc->Height = size.cy;
        pTargetDesc->Format = static_cast<DXGI_FORMAT>(format);

        pTargetDesc->MipLevels = 1;
        pTargetDesc->ArraySize = 1;
        pTargetDesc->SampleDesc.Count = 1;
        pTargetDesc->Usage = D3D11_USAGE_DEFAULT;
        pTargetDesc->BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        return S_OK;
    }

    HRESULT FrameRender::Prepare(SIZE targetSize, SIZE originSize, int format) {
        HRESULT hr;
        target_size_ = targetSize;
        // Create target texture
        D3D11_TEXTURE2D_DESC targetDesc;
        InitializeDesc(targetSize, &targetDesc, format);
        hr = m_Device->CreateTexture2D(&targetDesc, nullptr, &m_TargetTexture);
        RETURN_ON_BAD_HR(hr);

		D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));
		desc.Width = targetDesc.Width;
		desc.Height = targetDesc.Height;
		desc.Format = targetDesc.Format;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        hr = m_Device->CreateTexture2D(&desc, nullptr, &m_FinalTexture);
        if (hr != S_OK) {
            LOGE("Create final texture failed: {:x}", (uint32_t)hr);
            return hr;
        }

        targetDesc.Width = originSize.cx;
        targetDesc.Height = originSize.cy;
		hr = m_Device->CreateTexture2D(&targetDesc, nullptr, &m_SrcTexture);
        if (hr != S_OK) {
            LOGE("Create src texture failed: {:x}", (uint32_t)hr);
            return hr;
        }

		D3D11_TEXTURE2D_DESC srcDesc = { };
        m_SrcTexture->GetDesc(&srcDesc);
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = srcDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;
        hr = m_Device->CreateShaderResourceView(m_SrcTexture, &SRVDesc, &m_SrcSrv);
        if (hr != S_OK) {
            LOGE("Create shader texture failed: {:x}", (uint32_t)hr);
            return hr;
        }

        // Make new render target view
        hr = MakeRTV();
        RETURN_ON_BAD_HR(hr);

        // Set view port
        SetViewPort(targetSize);

        // Create the sample state
        D3D11_SAMPLER_DESC SampDesc;
        RtlZeroMemory(&SampDesc, sizeof(SampDesc));
        SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        SampDesc.MinLOD = 0;
        SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = m_Device->CreateSamplerState(&SampDesc, &m_SamplerLinear);
        RETURN_ON_BAD_HR(hr);

        // Create the blend state
        D3D11_BLEND_DESC BlendStateDesc;
        BlendStateDesc.AlphaToCoverageEnable = FALSE;
        BlendStateDesc.IndependentBlendEnable = FALSE;
        BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
        BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        hr = m_Device->CreateBlendState(&BlendStateDesc, &m_BlendState);
        RETURN_ON_BAD_HR(hr);

        // Initialize shaders
        hr = InitShaders();
        RETURN_ON_BAD_HR(hr);

        return S_OK;
    }

    HRESULT FrameRender::Draw() {
        FLOAT color[]{0, 0, 0, 0};
        m_DeviceContext->ClearRenderTargetView(m_RTV, color);

        HRESULT hr;

        // Vertices for drawing whole texture
        VERTEX Vertices[NUMVERTICES] =
        {
            {XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
            {XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
            {XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
            {XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
        };

        // Set resources
        UINT Stride = sizeof(VERTEX);
        UINT Offset = 0;
        FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
        m_DeviceContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);
        m_DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        m_DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
        m_DeviceContext->PSSetShaderResources(0, 1, &m_SrcSrv);
        m_DeviceContext->PSSetSamplers(0, 1, &m_SamplerLinear);
        m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        if (!VertexBuffer) {
			D3D11_BUFFER_DESC BufferDesc;
			RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.Usage = D3D11_USAGE_DEFAULT;
			BufferDesc.ByteWidth = sizeof(VERTEX) * NUMVERTICES;
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			BufferDesc.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA InitData;
			RtlZeroMemory(&InitData, sizeof(InitData));
			InitData.pSysMem = Vertices;

            hr = m_Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffer);
            if (FAILED(hr)) {
                printf("CreateBuffer failed !\n");
                return S_FALSE;
            }
        }
        m_DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);

        // Draw textured quad onto render target
        m_DeviceContext->Draw(NUMVERTICES, 0);
        m_DeviceContext->CopyResource(m_FinalTexture, m_TargetTexture);
        return S_OK;
    }

    HRESULT FrameRender::MakeRTV() {
        HRESULT hr = m_Device->CreateRenderTargetView(m_TargetTexture, nullptr, &m_RTV);
        RETURN_ON_BAD_HR(hr);
        m_DeviceContext->OMSetRenderTargets(1, &m_RTV, nullptr);
        return S_OK;
    }

    void FrameRender::SetViewPort(SIZE size) {
        D3D11_VIEWPORT VP;
        VP.Width = static_cast<FLOAT>(size.cx);
        VP.Height = static_cast<FLOAT>(size.cy);
        VP.MinDepth = 0.0f;
        VP.MaxDepth = 1.0f;
        VP.TopLeftX = 0;
        VP.TopLeftY = 0;
        m_DeviceContext->RSSetViewports(1, &VP);
    }

    HRESULT FrameRender::InitShaders() {
        HRESULT hr;

        UINT Size = ARRAYSIZE(g_VS);
        hr = m_Device->CreateVertexShader(g_VS, Size, nullptr, &m_VertexShader);
        RETURN_ON_BAD_HR(hr);

        D3D11_INPUT_ELEMENT_DESC Layout[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        UINT NumElements = ARRAYSIZE(Layout);
        hr = m_Device->CreateInputLayout(Layout, NumElements, g_VS, Size, &m_InputLayout);
        RETURN_ON_BAD_HR(hr);

        m_DeviceContext->IASetInputLayout(m_InputLayout);

        Size = ARRAYSIZE(g_PS);
        hr = m_Device->CreatePixelShader(g_PS, Size, nullptr, &m_PixelShader);
        RETURN_ON_BAD_HR(hr);

        return S_OK;
    }

    void FrameRender::CleanRefs() {
        if (m_VertexShader) {
            m_VertexShader->Release();
            m_VertexShader = nullptr;
        }

        if (m_PixelShader) {
            m_PixelShader->Release();
            m_PixelShader = nullptr;
        }

        if (m_InputLayout) {
            m_InputLayout->Release();
            m_InputLayout = nullptr;
        }

        if (m_RTV) {
            m_RTV->Release();
            m_RTV = nullptr;
        }

        if (m_SamplerLinear) {
            m_SamplerLinear->Release();
            m_SamplerLinear = nullptr;
        }

        if (m_BlendState) {
            m_BlendState->Release();
            m_BlendState = nullptr;
        }

        if (m_TargetTexture) {
            m_TargetTexture->Release();
            m_TargetTexture = nullptr;
        }

        if (m_SrcTexture) {
            m_SrcTexture->Release();
            m_SrcTexture = nullptr;
        }

        if (m_SrcSrv) {
            m_SrcSrv->Release();
            m_SrcSrv = nullptr;
        }

        if (m_DeviceContext) {
            m_DeviceContext->Release();
            m_DeviceContext = nullptr;
        }

        if (m_Device) {
            m_Device->Release();
            m_Device = nullptr;
        }

        if (VertexBuffer) {
            VertexBuffer->Release();
            VertexBuffer = nullptr;
        }

        if (m_SrcSrv) {
            m_SrcSrv->Release();
            m_SrcSrv = nullptr;
        }
    }

}