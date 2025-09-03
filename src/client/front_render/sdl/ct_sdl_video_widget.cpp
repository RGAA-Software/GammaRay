//
// Created by RGAA on 29/08/2025.
//

#include "ct_sdl_video_widget.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_util.h"

namespace tc
{

    SDLVideoWidget::SDLVideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                                   int dup_idx, RawImageFormat format, QWidget *parent)
            : VideoWidget(ctx, sdk, dup_idx) {
        this->context = ctx;
        this->format = format;

        if (SDL_Init(SDL_INIT_EVERYTHING)) {
            LOGE("Could not initialize SDL - {}", SDL_GetError());
            return;
        }
        screen = SDL_CreateWindowFrom((void*)((QWidget*)this->winId()));
        if (screen == NULL) {
            LOGE("Could not create window - {}", SDL_GetError());
            return;
        }
        sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
        if (!sdlRenderer) {
            LOGE("create renderer failed !");
            return;
        }

        grabKeyboard();

        LOGE("sdl widget init success. \n");

    }

    SDLVideoWidget::~SDLVideoWidget() {
        if (sdlRenderer) {
            SDL_DestroyRenderer(sdlRenderer);
        }
        if (screen) {
            SDL_DestroyWindow(screen);
        }
        SDL_Quit();

        printf("sdl widget exit ..... \n");
    }

    void SDLVideoWidget::Init(int frame_width, int frame_height) {
        if (init) {
            return;
        }
        init = true;

        this->frame_width = frame_width;
        this->frame_height = frame_height;

        sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, frame_width, frame_height);
        sdlRect.x = 0;
        sdlRect.y = 0;
        sdlRect.w = frame_width;
        sdlRect.h = frame_height;
    }

    void SDLVideoWidget::RefreshImage(const std::shared_ptr<RawImage> &image) {
        if (image->img_format == RawImageFormat::kRawImageI420) {
            this->RefreshImage(image);
        }
    }

    void SDLVideoWidget::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
        Init(image->img_width, image->img_height);

        int y_buf_size = image->img_width * image->img_height;
        int uv_buf_size = y_buf_size / 4;
        char* buf = image->Data();
        RefreshI420Buffer(buf, y_buf_size, // y
                          buf + y_buf_size, uv_buf_size, // u
                          buf + y_buf_size + uv_buf_size, uv_buf_size, // v
                          image->img_width, image->img_height
        );
    }

    void SDLVideoWidget::RefreshI420Buffer(const char* y_buf, int y_buf_size, const char* u_buf, int u_buf_size, const char* v_buf, int v_buf_size, int width, int height) {
        int ret = SDL_UpdateYUVTexture(sdlTexture, NULL,
                                       (uint8_t*)y_buf, width,
                                       (uint8_t*)u_buf, width/2,
                                       (uint8_t*)v_buf, width/2);
        ret = SDL_RenderClear(sdlRenderer);
        ret = SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);

        render_fps += 1;
        auto current_time = TimeUtil::GetCurrentTimestamp();
        if (current_time - last_update_fps_time >= 1000) {
            //statistics->render_fps = render_fps;
            render_fps = 0;
            last_update_fps_time = current_time;

            //statistics->streaming_time += 1;
        }
    }

    void SDLVideoWidget::resizeEvent(QResizeEvent* event) {
        QWidget::resizeEvent(event);

    }

    void SDLVideoWidget::mouseMoveEvent(QMouseEvent* e) {
        QWidget::mouseMoveEvent(e);
        VideoWidget::OnMouseMoveEvent(e, QWidget::width(), QWidget::height());
    }

    void SDLVideoWidget::mousePressEvent(QMouseEvent* e) {
        QWidget::mousePressEvent(e);
        VideoWidget::OnMousePressEvent(e, QWidget::width(), QWidget::height());
    }

    void SDLVideoWidget::mouseReleaseEvent(QMouseEvent* e) {
        QWidget::mouseReleaseEvent(e);
        VideoWidget::OnMouseReleaseEvent(e, QWidget::width(), QWidget::height());
    }

    void SDLVideoWidget::mouseDoubleClickEvent(QMouseEvent* e) {
        QWidget::mouseDoubleClickEvent(e);
        VideoWidget::OnMouseDoubleClickEvent(e);
    }

    void SDLVideoWidget::wheelEvent(QWheelEvent* e) {
        QWidget::wheelEvent(e);
        VideoWidget::OnWheelEvent(e, QWidget::width(), QWidget::height());
    }

    void SDLVideoWidget::keyPressEvent(QKeyEvent* e) {
        QWidget::keyPressEvent(e);
        VideoWidget::OnKeyPressEvent(e);
    }

    void SDLVideoWidget::keyReleaseEvent(QKeyEvent* e) {
        QWidget::keyReleaseEvent(e);
        VideoWidget::OnKeyReleaseEvent(e);
    }

    void SDLVideoWidget::closeEvent(QCloseEvent* event) {
        QWidget::closeEvent(event);
    }

    QWidget* SDLVideoWidget::AsWidget() {
        return dynamic_cast<QWidget*>(this);
    }

}