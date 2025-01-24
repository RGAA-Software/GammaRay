#pragma once

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <memory>
#include <map>

namespace tc
{

	class ClientContext;
    class NetMessage;
    class QtKeyConverter;
    class ThunderSdk;

    using OnMouseKeyboardEventCallback = std::function<void(int dup_idx, const std::shared_ptr<NetMessage>& msg)>;

    struct MouseEventDesc {
        int monitor_index = 0;
        float x_ratio = 0.0f;
        float y_ratio = 0.0f;
        int buttons = 0;
        int data = 0;
        int dx = 0;
        int dy = 0;
        bool pressed = false;
        bool released = false;
    };

	class VideoWidgetEvent {
	public:

		VideoWidgetEvent(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx);
		virtual ~VideoWidgetEvent();

		void OnWidgetResize(int w, int h);
		void OnMouseMoveEvent(QMouseEvent*, int widget_width, int widget_height);
		void OnMousePressEvent(QMouseEvent*, int widget_width, int widget_height);
		void OnMouseReleaseEvent(QMouseEvent*, int widget_width, int widget_height);
		void OnMouseDoubleClickEvent(QMouseEvent*);
		void OnWheelEvent(QWheelEvent* event, int widget_width, int widget_height);
		void OnKeyPressEvent(QKeyEvent* event);
		void OnKeyReleaseEvent(QKeyEvent* event);
        void RegisterMouseKeyboardEventCallback(const OnMouseKeyboardEventCallback& cbk);
        void SendKeyEvent(quint32 vk, bool down);

    private:
        void SendCallback(const std::shared_ptr<NetMessage>& msg);
        void SendMouseEvent(const MouseEventDesc& mouse_event);

    protected:
		std::shared_ptr<ClientContext> context_ = nullptr;
        std::shared_ptr<QtKeyConverter> key_converter_ = nullptr;
		int width = 0;
		int height = 0;
		int invalid_position = -10002200;
		int last_cursor_x_ = invalid_position;
		int last_cursor_y_ = invalid_position;
        OnMouseKeyboardEventCallback event_cbk_;
        std::shared_ptr<ThunderSdk> sdk_ = nullptr;
        int dup_idx_ = 0;
        int screen_size_ = 0;
	};

}