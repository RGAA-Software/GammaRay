#pragma once

#include <QWidget>
#include <memory>
#include "tc_message.pb.h"
#include "tc_client_sdk_new/gl/raw_image.h"
#include "client/front_render/ct_video_widget.h"

namespace tc
{
    class Data;
    class Sprite;
    class RawImage;
    class Director;
    class ClientContext;
    class ShaderProgram;
    class ThunderSdk;
    class Settings;
    class Thread;
    
    // Render vulkan img 
    class VulkanVideoWidget : public QWidget, public VideoWidget {
    public:
        VulkanVideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx, RawImageFormat format, QWidget* parent = nullptr);
        ~VulkanVideoWidget() override;
        QWidget* AsWidget() override;
        void RefreshImage(const std::shared_ptr<RawImage>& image) override;
        void OnTimer1S() override;
        WId GetRenderWId() override;
        QImage CaptureImage() override;
        std::string GetRenderTypeName() override {
            return "Vulkan";
        }
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
        QPaintEngine* paintEngine() const override;
        void paintEvent(QPaintEvent* event) override;
    private:
        void RefreshD3DImage(const std::shared_ptr<RawImage>& image);

    private:
        bool init = false;
    };

}