#pragma once

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <memory>
#include <map>
#include "sdk_messages.h"
#include "gl/raw_image.h"
#include "tc_common_new/fps_stat.h"

namespace tc
{

	class ClientContext;
    class NetMessage;
    class QtKeyConverter;
    class ThunderSdk;
    class Settings;
    class Thread;

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

	class VideoWidget {
	public:

		VideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx);
		virtual ~VideoWidget();

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
        RawImageFormat GetDisplayImageFormat();
        void SetDisplayImageFormat(RawImageFormat format);
        int GetCapturingMonitorWidth();
        int GetCapturingMonitorHeight();
        SdkCaptureMonitorInfo GetCaptureMonitorInfo();
        void RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo &mon_info);

        virtual QWidget* AsWidget();
        virtual void RefreshImage(const std::shared_ptr<RawImage> &image);
        virtual void OnTimer1S();

#ifdef WIN32

#endif

    private:
        void SendCallback(const std::shared_ptr<NetMessage>& msg);
        void SendMouseEvent(const MouseEventDesc& mouse_event);

    protected:
        Settings* settings_ = nullptr;
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
        SdkCaptureMonitorInfo cap_mon_info_{};
        std::shared_ptr<Thread> evt_cache_thread_ = nullptr;
        RawImageFormat raw_image_format_;
        FpsStat fps_stat_;
	};

}