#include "ct_test_vulkan_video_widget.h"

namespace tc {

	TestVulkanVideoWidget::TestVulkanVideoWidget() : QWidget() {
		setAttribute(Qt::WA_NativeWindow);
		setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | windowFlags());
		setGeometry(0, 0, 100, 100);
	}

	TestVulkanVideoWidget::~TestVulkanVideoWidget() {
	
	}
}