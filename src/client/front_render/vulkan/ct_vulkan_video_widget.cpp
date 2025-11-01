#include "ct_vulkan_video_widget.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/file.h"

namespace tc
{
    VulkanVideoWidget::VulkanVideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk,
        int dup_idx, RawImageFormat format, QWidget* parent)
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

    }

    VulkanVideoWidget::~VulkanVideoWidget() {

    }


    QPaintEngine* VulkanVideoWidget::paintEngine() const
    {
        return Q_NULLPTR;
    }

    void VulkanVideoWidget::paintEvent(QPaintEvent* event) {

    }

    void VulkanVideoWidget::RefreshImage(const std::shared_ptr<RawImage>& image) {
        
    }

    void VulkanVideoWidget::RefreshD3DImage(const std::shared_ptr<RawImage>& image) {
       
    }

    void VulkanVideoWidget::resizeEvent(QResizeEvent* event) {
        QWidget::resizeEvent(event);
        //render_mgr_->WindowResize();
    }

    void VulkanVideoWidget::mouseMoveEvent(QMouseEvent* e) {
        QWidget::mouseMoveEvent(e);
        VideoWidget::OnMouseMoveEvent(e, QWidget::width(), QWidget::height());
    }

    void VulkanVideoWidget::mousePressEvent(QMouseEvent* e) {
        QWidget::mousePressEvent(e);
        VideoWidget::OnMousePressEvent(e, QWidget::width(), QWidget::height());
    }

    void VulkanVideoWidget::mouseReleaseEvent(QMouseEvent* e) {
        QWidget::mouseReleaseEvent(e);
        VideoWidget::OnMouseReleaseEvent(e, QWidget::width(), QWidget::height());
    }

    void VulkanVideoWidget::mouseDoubleClickEvent(QMouseEvent* e) {
        QWidget::mouseDoubleClickEvent(e);
        VideoWidget::OnMouseDoubleClickEvent(e);
    }

    void VulkanVideoWidget::wheelEvent(QWheelEvent* e) {
        QWidget::wheelEvent(e);
        VideoWidget::OnWheelEvent(e, QWidget::width(), QWidget::height());
    }

    void VulkanVideoWidget::keyPressEvent(QKeyEvent* e) {
        QWidget::keyPressEvent(e);
        VideoWidget::OnKeyPressEvent(e);
    }

    void VulkanVideoWidget::keyReleaseEvent(QKeyEvent* e) {
        QWidget::keyReleaseEvent(e);
        VideoWidget::OnKeyReleaseEvent(e);
    }

    void VulkanVideoWidget::closeEvent(QCloseEvent* event) {
        //QWidget::closeEvent(event);
    }

    QWidget* VulkanVideoWidget::AsWidget() {
        return dynamic_cast<QWidget*>(this);
    }

    void VulkanVideoWidget::OnTimer1S() {
        // LOGI("D3D11 refresh FPS: {}", fps_stat_.value());
    }

    WId VulkanVideoWidget::GetRenderWId() {
        return this->winId();
    }

    QImage VulkanVideoWidget::CaptureImage() {
        return QImage();
        //return render_mgr_->SaveBackBufferToImage();
    }

}