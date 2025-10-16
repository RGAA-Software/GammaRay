#pragma once
#include <qwidget.h>
#include <qevent.h>
#include <qobject.h>

namespace tc { 

	class TestVulkanVideoWidget : public QWidget {
	public:
        TestVulkanVideoWidget();
		~TestVulkanVideoWidget();

        // QPaintEngine* paintEngine() const override { return nullptr; } 的作用是完全禁用Qt的绘制系统对这个widget的渲染。
        // 这样在拉伸的时候, 不会显示白色背景
        QPaintEngine* paintEngine() const override {
            return nullptr; // 必须的！
        }

        void paintEvent(QPaintEvent* event) override {
            // 可以完全空着，或者只是标记已处理
            event->accept();
        }
	};

}