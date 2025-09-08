//
// Created by RGAA on 3/09/2025.
//

#include "raw_sdl_widget.h"
#include "tc_common_new/log.h"
#include "d3d11_render_manager.h"
#include "gl/raw_image.h"
#include "tc_common_new/time_util.h"

namespace tc
{

    static D3D11RenderManager output;

    HWND getSDLWindowHWND(SDL_Window* window) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(window, &wmInfo)) {
            return wmInfo.info.win.window;
        }
        return nullptr;
    }

    RawSdlWidget::RawSdlWidget() {

        if (SDL_Init(SDL_INIT_EVERYTHING)) {
//        if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
            LOGE("Could not initialize SDL - {}", SDL_GetError());
            return;
        }

        window_ = SDL_CreateWindow("Test Window",
                                    0,
                                    0,
                                    1280,
                                    768,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        if (window_ == NULL) {
            LOGE("Could not create window - {}", SDL_GetError());
            return;
        }
//        sdlRenderer = SDL_CreateRenderer(window_, -1, 0);
//        if (!sdlRenderer) {
//            LOGE("create renderer failed !");
//            return;
//        }
        SDL_ShowWindow(window_);

    }

    void RawSdlWidget::RefreshImage(const std::shared_ptr<RawImage>& image) {
        if (image->Format() != RawImageFormat::kRawImageD3D11Texture) {
            return;
        }
        auto beg = TimeUtil::GetCurrentTimestamp();
        ComPtr<ID3D11Device> device = nullptr;
        image->texture_->GetDevice(&device);

        ComPtr<ID3D11DeviceContext> context = nullptr;
        device->GetImmediateContext(&context);

        Init(image->img_width, image->img_height, device, context);

        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.right = image->img_width;
        srcBox.bottom = image->img_height;
        srcBox.front = 0;
        srcBox.back = 1;
        context->CopySubresourceRegion(output.GetTexture().Get(), 0, 0, 0, 0, image->texture_.Get(), image->src_subresource_, &srcBox);

        bool Occluded = false;
        auto Ret = output.UpdateApplicationWindow(&Occluded);

        auto end = TimeUtil::GetCurrentTimestamp();
        //LOGI("Refresh image used: {}ms", (end - beg));
        //fps_stat_.Tick();
    }

    void RawSdlWidget::Init(int frame_width, int frame_height, ComPtr<ID3D11Device> device,  ComPtr<ID3D11DeviceContext> context) {
        if (init_) {
            return;
        }
        RECT DeskBounds;
        auto hwnd = getSDLWindowHWND(window_);
        if (!hwnd) {
            MessageBoxA(0, 0, 0, 0);
            return;
        }
        output.InitOutput(hwnd, RawImageFormat::kRawImageI420, frame_width, frame_height, device, context);

        bool Occluded = false;
        auto Ret = output.UpdateApplicationWindow(&Occluded);

        init_ = true;
    }

}