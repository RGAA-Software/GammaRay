//
// Created by RGAA on 29/08/2025.
//

#include <QSurface>
#include "ct_d3d11_video_widget.h"
#include "tc_common_new/log.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/time_util.h"
#include "d3d11_render_manager.h"
#include "raw_sdl_widget.h"

namespace tc
{
//    struct NV12Frame
//    {
//        UINT width;
//        UINT height;
//        UINT pitch;
//        BYTE *Y;
//        BYTE *UV;
//    };
//
//    static NV12Frame* nv12_frame = nullptr;
//
//    static NV12Frame* ReadNV12FromFile()
//    {
//
//        char buf[1024];
//        FILE *file = nullptr;
//        sprintf_s(buf, "1920_1080.nv12");
//        //sprintf_s(buf, "content\\16.nv12");
//
//        fopen_s(&file, buf, "rb");
//
//        int size = sizeof(NV12Frame);
//        NV12Frame *nv12Frame = (NV12Frame*)malloc(size);
//        //int readBytes = fread(nv12Frame, size, 1, file);
//
//        nv12Frame->width = 1920;
//        nv12Frame->height = 1080;
//        nv12Frame->pitch = nv12Frame->width;
//
//        int readBytes = 0;
//
//        size = nv12Frame->pitch * nv12Frame->height;
//
//        //size = nv12Frame->pitch * nv12Frame->height;
//        nv12Frame->Y = (BYTE *)malloc(size);
//        readBytes = fread(nv12Frame->Y, size, 1, file);
//
//        size = nv12Frame->pitch * nv12Frame->height / 2;
//        nv12Frame->UV = (BYTE *)malloc(size);
//        readBytes = fread(nv12Frame->UV, size, 1, file);
//
//        fclose(file);
//
//        return nv12Frame;
//    }
//
//    static void WriteNV12ToTexture(NV12Frame *nv12Frame)
//    {
//        // Copy from CPU access texture to bitmap buffer
//        D3D11_MAPPED_SUBRESOURCE resource;
//        UINT subresource = D3D11CalcSubresource(0, 0, 0);
//        output.m_DeviceContext->Map(output.m_texture, subresource, D3D11_MAP_WRITE_DISCARD, 0, &resource);
//
//        BYTE* dptr = reinterpret_cast<BYTE*>(resource.pData);
//
//        for (int i = 0; i < nv12Frame->height; i++)
//        {
//            memcpy(dptr + resource.RowPitch * i, nv12Frame->Y + nv12Frame->pitch * i, nv12Frame->pitch);
//        }
//
//        for (int i = 0; i < nv12Frame->height / 2; i++)
//        {
//            memcpy(dptr + resource.RowPitch *(nv12Frame->height + i), nv12Frame->UV + nv12Frame->pitch * i, nv12Frame->pitch);
//        }
//
//        output.m_DeviceContext->Unmap(output.m_texture, subresource);
//    }

    D3D11VideoWidget::D3D11VideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                                   int dup_idx, RawImageFormat format, QWidget *parent)
            : QWidget(parent), VideoWidget(ctx, sdk, dup_idx) {
        this->context = ctx;
        this->format = format;
        
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Qt::black);
        setAutoFillBackground(true);
        setPalette(pal);

        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_NativeWindow);

        // Setting these attributes to our widget and returning null on paintEngine event
        // tells Qt that we'll handle all drawing and updating the widget ourselves.
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);

        setMouseTracking(true);
        grabKeyboard();

        render_mgr_ = std::make_shared<D3D11RenderManager>();

        ///
        // raw_sdl_widget_ = new RawSdlWidget();

    }

    D3D11VideoWidget::~D3D11VideoWidget() {

    }

    void D3D11VideoWidget::InitD3DEnv(int fw, int fh, ComPtr<ID3D11Device> device,  ComPtr<ID3D11DeviceContext> context) {
        if (init) {
            return;
        }

        if (auto r = render_mgr_->InitOutput((HWND)winId(), fw, fh, device, context);
            r == DUPL_RETURN_SUCCESS) {
            this->init = true;
            this->frame_width = fw;
            this->frame_height = fh;
        }
        else {
            LOGI("D3D output init failed: {}", (int)r);
            return;
        }

//        nv12_frame = ReadNV12FromFile();
//
//        WriteNV12ToTexture(nv12_frame);
//        free(nv12_frame->Y);
//        free(nv12_frame->UV);
//        free(nv12_frame);

//        bool Occluded = false;
//        auto r = render_mgr_->UpdateApplicationWindow(&Occluded);
    }

    QPaintEngine * D3D11VideoWidget::paintEngine() const
    {
        return Q_NULLPTR;
    }

    void D3D11VideoWidget::paintEvent(QPaintEvent * event) {

    }

    void D3D11VideoWidget::RefreshImage(const std::shared_ptr<RawImage>& image) {
        if (image->Format() != RawImageFormat::kRawImageD3D11Texture || !image->texture_) {
            return;
        }

        auto beg = TimeUtil::GetCurrentTimestamp();
        ComPtr<ID3D11Device> device = nullptr;
        image->texture_->GetDevice(&device);
        if (!device) {
            LOGE("No device with texture");
            return;
        }

        ComPtr<ID3D11DeviceContext> device_context = nullptr;
        device->GetImmediateContext(&device_context);
        if (!device_context) {
            LOGE("No device context with texture");
            return;
        }

        InitD3DEnv(image->img_width, image->img_height, device, device_context);

        if (this->frame_width != image->img_width || this->frame_height != image->img_height) {
            if (render_mgr_->RecreateTexture(image->img_width, image->img_height) == DUPL_RETURN_SUCCESS) {
                this->frame_width = image->img_width;
                this->frame_height = image->img_height;
            }
            else {
                return;
            }
        }

        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.right = image->img_width;
        srcBox.bottom = image->img_height;
        srcBox.front = 0;
        srcBox.back = 1;
        device_context->CopySubresourceRegion(render_mgr_->m_texture, 0, 0, 0, 0, image->texture_, image->src_subresource_, &srcBox);

        bool Occluded = false;
        auto Ret = render_mgr_->UpdateApplicationWindow(&Occluded);

        auto end = TimeUtil::GetCurrentTimestamp();
        //LOGI("Refresh image used: {}ms", (end - beg));
        fps_stat_.Tick();

        // For testing
        // raw_sdl_widget_->RefreshImage(image);
    }

    void D3D11VideoWidget::resizeEvent(QResizeEvent* event) {
        QWidget::resizeEvent(event);
        render_mgr_->WindowResize();
    }

    void D3D11VideoWidget::mouseMoveEvent(QMouseEvent* e) {
        QWidget::mouseMoveEvent(e);
        VideoWidget::OnMouseMoveEvent(e, QWidget::width(), QWidget::height());
    }

    void D3D11VideoWidget::mousePressEvent(QMouseEvent* e) {
        QWidget::mousePressEvent(e);
        VideoWidget::OnMousePressEvent(e, QWidget::width(), QWidget::height());
    }

    void D3D11VideoWidget::mouseReleaseEvent(QMouseEvent* e) {
        QWidget::mouseReleaseEvent(e);
        VideoWidget::OnMouseReleaseEvent(e, QWidget::width(), QWidget::height());
    }

    void D3D11VideoWidget::mouseDoubleClickEvent(QMouseEvent* e) {
        QWidget::mouseDoubleClickEvent(e);
        VideoWidget::OnMouseDoubleClickEvent(e);
    }

    void D3D11VideoWidget::wheelEvent(QWheelEvent* e) {
        QWidget::wheelEvent(e);
        VideoWidget::OnWheelEvent(e, QWidget::width(), QWidget::height());
    }

    void D3D11VideoWidget::keyPressEvent(QKeyEvent* e) {
        QWidget::keyPressEvent(e);
        VideoWidget::OnKeyPressEvent(e);
    }

    void D3D11VideoWidget::keyReleaseEvent(QKeyEvent* e) {
        QWidget::keyReleaseEvent(e);
        VideoWidget::OnKeyReleaseEvent(e);
    }

    void D3D11VideoWidget::closeEvent(QCloseEvent* event) {
        //QWidget::closeEvent(event);
    }

    QWidget* D3D11VideoWidget::AsWidget() {
        return dynamic_cast<QWidget*>(this);
    }

    void D3D11VideoWidget::OnTimer1S() {
        // LOGI("D3D11 refresh FPS: {}", fps_stat_.value());
    }

}