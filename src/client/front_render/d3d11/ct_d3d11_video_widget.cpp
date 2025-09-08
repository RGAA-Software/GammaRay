//
// Created by RGAA on 29/08/2025.
//

#include <QSurface>
#include "ct_d3d11_video_widget.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/file.h"
#include "d3d11_render_manager.h"
#include "raw_sdl_widget.h"

namespace tc
{


    D3D11VideoWidget::D3D11VideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                                   int dup_idx, RawImageFormat format, QWidget *parent)
            : QWidget(parent), VideoWidget(ctx, sdk, dup_idx) {
        this->raw_image_format_ = format;
        
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

        //nv12_frame = ReadNV12FromFile();

    }

    D3D11VideoWidget::~D3D11VideoWidget() {

    }

    bool D3D11VideoWidget::InitD3DEnvIfNeeded(RawImageFormat raw_format, int fw, int fh, ComPtr<ID3D11Device> device,  ComPtr<ID3D11DeviceContext> device_context) {
        if (init) {
            return true;
        }

        // Ignore to initialize when the Window was hidden or too small
        if (this->isHidden() || this->size().width() <= 256) {
            return false;
        }

        if (auto r = render_mgr_->InitOutput((HWND)winId(), raw_format, fw, fh, device, device_context);
            r == DUPL_RETURN_SUCCESS) {
            this->init = true;
            this->tex_width_ = fw;
            this->tex_height_ = fh;
        }
        else {
            LOGI("D3D output init failed: {}", (int)r);
            return false;
        }
        return true;
    }

    QPaintEngine * D3D11VideoWidget::paintEngine() const
    {
        return Q_NULLPTR;
    }

    void D3D11VideoWidget::paintEvent(QPaintEvent * event) {

    }

    void D3D11VideoWidget::RefreshImage(const std::shared_ptr<RawImage>& image) {
        auto beg = TimeUtil::GetCurrentTimestamp();

        if (!image->device_) {
            LOGE("No device with texture");
            return;
        }

        if (!image->device_context_) {
            LOGE("No device context with texture");
            return;
        }

        if (!InitD3DEnvIfNeeded(image->Format(), image->img_width, image->img_height, image->device_, image->device_context_)) {
            LOGE("Don't have d3d environment.");
            return;
        }

        // LOGI("this image format: {}, image in: {}", this->raw_image_format_, image->img_format);
        if (this->tex_width_ != image->img_width || this->tex_height_ != image->img_height || this->raw_image_format_ != image->img_format) {
            if (render_mgr_->RecreateTexture(image->img_format, image->img_width, image->img_height) == DUPL_RETURN_SUCCESS) {
                this->tex_width_ = image->img_width;
                this->tex_height_ = image->img_height;
                this->raw_image_format_ = image->img_format;
            }
            else {
                LOGE("Recreate Texture failed!");
                return;
            }
        }

        if (image->Format() == RawImageFormat::kRawImageD3D11Texture) {
            auto texture = render_mgr_->GetTexture();
            if (!texture) {
                LOGE("Don't have a valid texture to draw.");
                return;
            }
            if (!image->texture_) {
                LOGE("The image format is D3D11Texture, but there isn't a valid texture.");
                return;
            }
            this->RefreshD3DImage(image);
        }
        else {
            // Flush to GPU memory directly
            if (image->Format() == RawImageFormat::kRawImageI420  || raw_image_format_ == RawImageFormat::kRawImageI444) {
                // Y
                {
                    D3D11_MAPPED_SUBRESOURCE resource;
                    UINT subresource = D3D11CalcSubresource(0, 0, 0);
                    image->device_context_->Map(render_mgr_->GetYPlane().Get(), subresource, D3D11_MAP_WRITE_DISCARD, 0, &resource);
                    BYTE* dptr = reinterpret_cast<BYTE*>(resource.pData);
                    for (int i = 0; i < image->img_height; i++) {
                        memcpy(dptr + resource.RowPitch * i, image->Data() + image->img_width * i, image->img_width);
                    }
                    image->device_context_->Unmap(render_mgr_->GetYPlane().Get(), subresource);
                }

                // U
                {
                    int y_offset = image->img_width * image->img_height;
                    D3D11_MAPPED_SUBRESOURCE resource;
                    UINT subresource = D3D11CalcSubresource(0, 0, 0);
                    image->device_context_->Map(render_mgr_->GetUPlane().Get(), subresource, D3D11_MAP_WRITE_DISCARD, 0, &resource);
                    BYTE* dptr = reinterpret_cast<BYTE*>(resource.pData);
                    if (raw_image_format_ == RawImageFormat::kRawImageI420) {
                        for (int i = 0; i < image->img_height / 2; i++) {
                            memcpy(dptr + resource.RowPitch * i, image->Data() + y_offset + image->img_width / 2 * i, image->img_width / 2);
                        }
                    }
                    else if (raw_image_format_ == RawImageFormat::kRawImageI444) {
                        for (int i = 0; i < image->img_height; i++) {
                            memcpy(dptr + resource.RowPitch * i, image->Data() + y_offset + image->img_width * i, image->img_width);
                        }
                    }
                    image->device_context_->Unmap(render_mgr_->GetUPlane().Get(), subresource);
                }

                // V
                {
                    D3D11_MAPPED_SUBRESOURCE resource;
                    UINT subresource = D3D11CalcSubresource(0, 0, 0);
                    image->device_context_->Map(render_mgr_->GetVPlane().Get(), subresource, D3D11_MAP_WRITE_DISCARD, 0, &resource);
                    BYTE* dptr = reinterpret_cast<BYTE*>(resource.pData);
                    if (raw_image_format_ == RawImageFormat::kRawImageI420) {
                        int y_u_offset = image->img_width * image->img_height * (1 + 1.0f/4);
                        for (int i = 0; i < image->img_height / 2; i++) {
                            memcpy(dptr + resource.RowPitch * i, image->Data() + y_u_offset + image->img_width / 2 * i, image->img_width / 2);
                        }
                    }
                    else if (raw_image_format_ == RawImageFormat::kRawImageI444) {
                        int y_u_offset = image->img_width * image->img_height * 2;
                        for (int i = 0; i < image->img_height; i++) {
                            memcpy(dptr + resource.RowPitch * i, image->Data() + y_u_offset + image->img_width * i, image->img_width);
                        }
                    }
                    image->device_context_->Unmap(render_mgr_->GetVPlane().Get(), subresource);
                }
            }

            bool Occluded = false;
            auto Ret = render_mgr_->UpdateApplicationWindow(&Occluded);
        }

        auto end = TimeUtil::GetCurrentTimestamp();
        //LOGI("Refresh image used: {}ms", (end - beg));
        fps_stat_.Tick();

        // For testing
        // raw_sdl_widget_->RefreshImage(image);
    }

    void D3D11VideoWidget::RefreshD3DImage(const std::shared_ptr<RawImage>& image) {
        D3D11_BOX srcBox;
        srcBox.left = 0;
        srcBox.top = 0;
        srcBox.right = image->img_width;
        srcBox.bottom = image->img_height;
        srcBox.front = 0;
        srcBox.back = 1;
        image->device_context_->CopySubresourceRegion(render_mgr_->GetTexture().Get(), 0, 0, 0, 0, image->texture_.Get(), image->src_subresource_, &srcBox);

        bool Occluded = false;
        auto Ret = render_mgr_->UpdateApplicationWindow(&Occluded);
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