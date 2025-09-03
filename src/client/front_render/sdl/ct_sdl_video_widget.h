//
// Created by RGAA on 29/08/2025.
//

#ifndef GAMMARAYPREMIUM_CT_SDL_VIDEO_WIDGET_H
#define GAMMARAYPREMIUM_CT_SDL_VIDEO_WIDGET_H

#include <QWidget>
#include <memory>
#include "tc_message.pb.h"
#include "tc_client_sdk_new/gl/raw_image.h"
#include "client/front_render/ct_video_widget_event.h"

#include <SDL2/SDL.h>

// !!! Almost the same with OpenGL version, for Testing !!!
namespace tc
{

    class Data;
    class Sprite;
    class RawImage;
    class Director;
    class ClientContext;
    class ShaderProgram;
    class Statistics;
    class ThunderSdk;
    class Settings;

    class SDLVideoWidget : public QWidget, public VideoWidget {
    public:
        SDLVideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                       int dup_idx, RawImageFormat format, QWidget *parent = nullptr);
        ~SDLVideoWidget() override;

        void Init(int frame_width, int frame_height);
        QWidget* AsWidget() override;
        void RefreshImage(const std::shared_ptr<RawImage> &image) override;

    protected:
        void mouseMoveEvent(QMouseEvent*) override;
        void mousePressEvent(QMouseEvent*) override;
        void mouseReleaseEvent(QMouseEvent*) override;
        void resizeEvent(QResizeEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent*) override;
        void wheelEvent(QWheelEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
        void closeEvent(QCloseEvent* event) override;

    private:
        void RefreshI420Image(const std::shared_ptr<RawImage>& image);
        void RefreshI420Buffer(const char* y_buf, int y_buf_size, const char* u_buf, int u_buf_size, const char* v_buf, int v_buf_size, int width, int height);

    private:
        std::shared_ptr<ClientContext> context = nullptr;
        int frame_width = 0;
        int frame_height = 0;
        RawImageFormat format;

        SDL_Window* screen = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
        SDL_Texture* sdlTexture = nullptr;
        SDL_Rect sdlRect;

        bool init = false;

        int render_fps = 0;
        uint64_t last_update_fps_time = 0;

        Statistics* statistics = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_CT_SDL_VIDEO_WIDGET_H
