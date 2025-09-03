//
// Created by RGAA on 29/08/2025.
//

#ifndef GAMMARAYPREMIUM_CT_D3D11_VIDEO_WIDGET_H
#define GAMMARAYPREMIUM_CT_D3D11_VIDEO_WIDGET_H

#include <QWidget>
#include <memory>
#include "tc_message.pb.h"
#include "tc_client_sdk_new/gl/raw_image.h"
#include "client/front_render/ct_video_widget_event.h"

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
    class Thread;
    class D3D11RenderManager;
    // for testing
    class RawSdlWidget;

    // Render DXGI TEXTURE
    class D3D11VideoWidget : public QWidget, public VideoWidget {
    public:
        D3D11VideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                       int dup_idx, RawImageFormat format, QWidget *parent = nullptr);
        ~D3D11VideoWidget() override;

        void InitD3DEnv(int frame_width, int frame_height, ComPtr<ID3D11Device>,  ComPtr<ID3D11DeviceContext>);
        QWidget * AsWidget() override;
        void RefreshImage(const std::shared_ptr<RawImage>& image) override;
        void OnTimer1S() override;

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
        QPaintEngine * paintEngine() const override;
        void paintEvent(QPaintEvent *event) override;

    private:
        std::shared_ptr<ClientContext> context = nullptr;
        int frame_width = 0;
        int frame_height = 0;
        RawImageFormat format;

        bool init = false;

        int render_fps = 0;
        uint64_t last_update_fps_time = 0;

        Statistics* statistics = nullptr;

        std::shared_ptr<D3D11RenderManager> render_mgr_ = nullptr;

        // for testing
        //RawSdlWidget* raw_sdl_widget_ = nullptr;

    };

}

#endif //GAMMARAYPREMIUM_CT_SDL_VIDEO_WIDGET_H
