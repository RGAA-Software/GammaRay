#include "ct_video_widget.h"

#include "tc_message.pb.h"
#include "ct_qt_key_converter.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/thread.h"
#include "tc_client_sdk_new/thunder_sdk.h"
#include "client/ct_client_context.h"
#include "client/ct_app_message.h"
#include "client/ct_settings.h"
#include "tc_common_new/time_util.h"
#include "tc_message_new/proto_converter.h"
#ifdef WIN32
#include <Windows.h>
#endif
#include <qdebug.h>

namespace tc
{

	VideoWidget::VideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx) {
        TimeDuration dr("VideoWidget");
		this->context_ = ctx;
        // to do, dup_idx_ 已经废弃，待删除
        this->dup_idx_ = dup_idx;
        this->key_converter_ = std::make_shared<QtKeyConverter>();
        this->sdk_ = sdk;
        this->settings_ = Settings::Instance();
        this->evt_cache_thread_ = Thread::Make("evt_cache_thread", 64);
        this->evt_cache_thread_->Poll();
	}

	VideoWidget::~VideoWidget() = default;

	void VideoWidget::OnWidgetResize(int w, int h) {
		this->widget_width_ = w;
		this->widget_height_ = h;
	}

	void VideoWidget::OnMouseMoveEvent(QMouseEvent* event, int widget_width, int widget_height) {
        auto curr_pos = event->pos();
        MouseEventDesc mouse_event_desc;
        mouse_event_desc.buttons = 0;
        mouse_event_desc.buttons |= ButtonFlag::kMouseMove;
        mouse_event_desc.x_ratio = ((float)curr_pos.x()) / ((float)(widget_width));
        mouse_event_desc.y_ratio = ((float)curr_pos.y()) / ((float)(widget_height));

        if (last_cursor_x_ != invalid_position && last_cursor_y_ != invalid_position) {
            mouse_event_desc.dx = curr_pos.x() - last_cursor_x_;
            mouse_event_desc.dy = curr_pos.y() - last_cursor_y_;
        }

        last_cursor_x_ = curr_pos.x();
        last_cursor_y_ = curr_pos.y();
        SendMouseEvent(mouse_event_desc);
	}

	void VideoWidget::OnMousePressEvent(QMouseEvent* event, int widget_width, int widget_height) {
        auto curr_pos = event->pos();
        MouseEventDesc mouse_event_desc;
        mouse_event_desc.buttons = 0;
        auto pressed_button = 0;
        if(event->button() == Qt::LeftButton) {
            pressed_button = ButtonFlag::kLeftMouseButtonDown;
        }
        if(event->button() == Qt::RightButton) {
            pressed_button = ButtonFlag::kRightMouseButtonDown;
        }
        if(event->button() == Qt::MiddleButton) {
            pressed_button = ButtonFlag::kMiddleMouseButtonDown;
        }

        mouse_event_desc.buttons = pressed_button;
        mouse_event_desc.pressed = true;
        mouse_event_desc.x_ratio = ((float)curr_pos.x()) / ((float)(widget_width));
        mouse_event_desc.y_ratio = ((float)curr_pos.y()) / ((float)(widget_height));
        SendMouseEvent(mouse_event_desc);

        context_->SendAppMessage(MsgClientMousePressed {

        });
	}

	void VideoWidget::OnMouseReleaseEvent(QMouseEvent* event, int widget_width, int widget_height) {
        auto curr_pos = event->pos();
        MouseEventDesc mouse_event_desc;
        auto released_button = 0;
        if (event->button() == Qt::LeftButton) {
            released_button = ButtonFlag::kLeftMouseButtonUp;
        }
        if (event->button() == Qt::RightButton) {
            released_button = ButtonFlag::kRightMouseButtonUp;
        }
        if (event->button() == Qt::MiddleButton) {
            released_button = ButtonFlag::kMiddleMouseButtonUp;
        }
        mouse_event_desc.buttons = released_button;
        mouse_event_desc.released = true;
        mouse_event_desc.x_ratio = ((float)curr_pos.x()) / ((float)(widget_width));
        mouse_event_desc.y_ratio = ((float)curr_pos.y()) / ((float)(widget_height));
        SendMouseEvent(mouse_event_desc);
	}

	void VideoWidget::OnMouseDoubleClickEvent(QMouseEvent*) {
	}

	void VideoWidget::OnWheelEvent(QWheelEvent* event, int widget_width, int widget_height) {
        MouseEventDesc mouse_event_desc;
        mouse_event_desc.buttons = 0;
        mouse_event_desc.x_ratio = ((float)last_cursor_x_) / ((float)(widget_width));
        mouse_event_desc.y_ratio = ((float)last_cursor_y_) / ((float)(widget_height));
        QPoint angle_delta = event->angleDelta();
        QPoint numDegrees = event->angleDelta() / 8;
        if (!numDegrees.isNull()) {
            mouse_event_desc.buttons = ButtonFlag::kMouseEventWheel;
            if(angle_delta.x() != 0) {
                mouse_event_desc.data = angle_delta.x();
            }
            if(angle_delta.y() != 0) {
                mouse_event_desc.data = angle_delta.y();
            }
            SendMouseEvent(mouse_event_desc);
        }
	}

	void VideoWidget::OnKeyPressEvent(QKeyEvent* e) {
#ifdef WIN32
        SendKeyEvent(e->nativeVirtualKey(), true);
#endif
	}

	void VideoWidget::OnKeyReleaseEvent(QKeyEvent* e) {
#ifdef WIN32
        SendKeyEvent(e->nativeVirtualKey(), false);
#endif
	}

    void VideoWidget::RegisterMouseKeyboardEventCallback(const OnMouseKeyboardEventCallback& cbk) {
        event_cbk_ = cbk;
    }

    void VideoWidget::SendCallback(const std::shared_ptr<NetMessage>& msg) {
        if (event_cbk_) {
            event_cbk_(dup_idx_, msg);
        }
    }

    void VideoWidget::SendKeyEvent(quint32 vk, bool down) {
        if (settings_->only_viewing_) {
            return;
        }
        //LOGI("*VK: 0x{:x}, down: {}", vk, down);
        short num_lock_state = -1;
        if (vk >= VK_NUMPAD0 && vk <= VK_DIVIDE || vk == VK_NUMLOCK
            || vk == VK_HOME || vk == VK_END || vk == VK_PRIOR || vk == VK_NEXT
            || vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT
            || vk == VK_INSERT || vk == VK_DELETE ) {
            num_lock_state = GetKeyState(VK_NUMLOCK);
        }

        short caps_lock_state = -1;
        if (vk >= 0x41 && vk <= 0x5A) {
            caps_lock_state = GetKeyState(VK_CAPITAL);
        }

        std::map<int, bool> sys_key_status = key_converter_->GetSysKeyStatus();
        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kKeyEvent);
        msg->set_device_id(settings_->device_id_);
        msg->set_stream_id(settings_->stream_id_);
        auto key_event = new tc::KeyEvent();
        key_event->set_down(down);
        key_event->set_key_code(vk);
        key_event->set_num_lock_status(num_lock_state);
        key_event->set_caps_lock_status(caps_lock_state);
        if (num_lock_state != -1) {
            key_event->set_status_check(tc::KeyEvent::kCheckNumLock);
        } else if (caps_lock_state != -1) {
            key_event->set_status_check(tc::KeyEvent::kCheckCapsLock);
        } else {
            key_event->set_status_check(tc::KeyEvent::kDontCareLockKey);
        }
        auto cur_time = GetCurrentTime();
        key_event->set_timestamp(cur_time);
        msg->set_allocated_key_event(key_event);

        if (auto buffer = tc::ProtoAsData(msg); buffer && sdk_) {
            this->sdk_->PostMediaMessage(buffer);
        }
    }

    void VideoWidget::SendMouseEvent(const MouseEventDesc& mouse_event_desc) {
        if (!sdk_ || settings_->only_viewing_) {
            return;
        }

        auto msg = std::make_shared<Message>();
        msg->set_type(tc::kMouseEvent);
        msg->set_device_id(settings_->device_id_);
        msg->set_stream_id(settings_->stream_id_);
        auto mouse_event = new tc::MouseEvent();
        mouse_event->set_x_ratio(mouse_event_desc.x_ratio);
        mouse_event->set_y_ratio(mouse_event_desc.y_ratio);
        mouse_event->set_button(mouse_event_desc.buttons);
        auto cur_time = GetCurrentTime();
        mouse_event->set_timestamp(cur_time);
        mouse_event->set_monitor_name(cap_mon_info_.mon_name_);
        mouse_event->set_data(mouse_event_desc.data);
        mouse_event->set_delta_x(mouse_event_desc.dx);
        mouse_event->set_delta_y(mouse_event_desc.dy);
        mouse_event->set_pressed(mouse_event_desc.pressed);
        mouse_event->set_released(mouse_event_desc.released);
        msg->set_allocated_mouse_event(mouse_event);

        this->evt_cache_thread_->Post([=, this]() {
            auto queuing_count = this->sdk_->GetQueuingMediaMsgCount();
            while (queuing_count > 16) {
                LOGI("queuing too many mouse event: {}, cache thread tasks: {}", queuing_count, evt_cache_thread_->TaskSize());
                TimeUtil::DelayBySleep(1);
                queuing_count = this->sdk_->GetQueuingMediaMsgCount();
            }
            if (auto buffer = tc::ProtoAsData(msg); buffer && sdk_) {
                sdk_->PostMediaMessage(buffer);
            }
        });
    }

    void VideoWidget::RefreshImage(const std::shared_ptr<RawImage> &image) {
        if (image->Format() == RawImageFormat::kRawImageI420) {
            this->RefreshI420Image(image);
        }
        else if (image->Format() == RawImageFormat::kRawImageI444) {
            this->RefreshI444Image(image);
        }
        else if (image->Format() == RawImageFormat::kRawImageRGB) {
            this->RefreshRGBBuffer(image->Data(), image->img_width, image->img_height, image->img_ch);
        }
    }

    RawImageFormat VideoWidget::GetDisplayImageFormat() {
        return raw_image_format_;
    }

    void VideoWidget::SetDisplayImageFormat(RawImageFormat format) {
        raw_image_format_ = format;
    }

    void VideoWidget::RefreshCapturedMonitorInfo(const SdkCaptureMonitorInfo& mon_info) {
        cap_mon_info_ = mon_info;
    }

    int VideoWidget::GetCapturingMonitorWidth() {
        return cap_mon_info_.frame_width_;
    }

    int VideoWidget::GetCapturingMonitorHeight() {
        return cap_mon_info_.frame_height_;
    }

    SdkCaptureMonitorInfo VideoWidget::GetCaptureMonitorInfo() {
        return cap_mon_info_;
    }

    void VideoWidget::OnTimer1S() {

    }

    QWidget* VideoWidget::AsWidget() {
        return nullptr;
    }

    void VideoWidget::RefreshRGBBuffer(const char* buf, int width, int height, int channel) {
        std::lock_guard<std::mutex> guard(buf_mtx_);
        int size = width * height * channel;
        if (!rgb_buffer_) {
            rgb_buffer_ = Data::Make(nullptr, size);
        }
        if (tex_width_ != width || tex_height_ != height) {
            need_create_texture_ = true;
        }
        memcpy(rgb_buffer_->DataAddr(), buf, size);
        tex_width_ = width;
        tex_height_ = height;
        tex_channel_ = channel;

        this->OnUpdate();
    }

    void VideoWidget::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
        std::lock_guard<std::mutex> guard(buf_mtx_);
        int y_buf_size = image->img_width * image->img_height;
        int uv_buf_size = y_buf_size / 4;
        char* buf = image->Data();
        RefreshI420Buffer(buf, y_buf_size,
                          buf + y_buf_size, uv_buf_size,
                          buf + y_buf_size + uv_buf_size, uv_buf_size,
                          image->img_width, image->img_height
        );
    }

    void VideoWidget::RefreshI420Buffer(const char* y_buf, int y_buf_size,
                                              const char* u_buf, int u_buf_size,
                                              const char* v_buf, int v_buf_size,
                                              int width, int height) {
        auto target_y_size = width * height;
        auto target_u_size = width/2 * height/2;
        if (!y_buffer_ || y_buffer_->Size() != y_buf_size) {
            y_buffer_ = Data::Make(y_buf, y_buf_size);
            need_create_texture_ = true;
        }
        if (!u_buffer_ || u_buffer_->Size() != u_buf_size) {
            u_buffer_ = Data::Make(u_buf, u_buf_size);
            need_create_texture_ = true;
        }
        if (!v_buffer_ || v_buffer_->Size() != v_buf_size) {
            v_buffer_ = Data::Make(v_buf, v_buf_size);
            need_create_texture_ = true;
        }

        if (tex_width_ != width || tex_height_ != height) {
            need_create_texture_ = true;
        }
        memcpy(y_buffer_->DataAddr(), y_buf, y_buf_size);
        memcpy(u_buffer_->DataAddr(), u_buf, u_buf_size);
        memcpy(v_buffer_->DataAddr(), v_buf, v_buf_size);

        tex_width_ = width;
        tex_height_ = height;

        this->OnUpdate();
    }

    void VideoWidget::RefreshI444Image(const std::shared_ptr<RawImage>& image) {
        std::lock_guard<std::mutex> guard(buf_mtx_);
        int y_buf_size = image->img_width * image->img_height;
        int uv_buf_size = y_buf_size;
        char* buf = image->Data();
#if 0   // debug: save yuv file
        {
			std::string img_data;
			img_data.resize(image->Size());
			memcpy(img_data.data(), buf, image->Size());
			static int index = 0;
			auto yuv444_file = File::OpenForWrite("RefreshI444Image_" + std::to_string(index % 10) + ".yuv444");
			if (yuv444_file) {
				yuv444_file->Write(0, img_data);
			}
			++index;
		}
#endif
        RefreshI444Buffer(
                buf, y_buf_size,
                buf + y_buf_size, uv_buf_size,
                buf + y_buf_size + uv_buf_size, uv_buf_size,
                image->img_width, image->img_height
        );
    }

    void VideoWidget::RefreshI444Buffer(const char* y_buf, int y_buf_size,
                                              const char* u_buf, int u_buf_size,
                                              const char* v_buf, int v_buf_size,
                                              int width, int height) {
        auto target_y_size = width * height;
        auto target_u_size = width * height;

        if (!y_buffer_ || y_buffer_->Size() != y_buf_size) {
            y_buffer_ = Data::Make(y_buf, y_buf_size);
            need_create_texture_ = true;
        }
        if (!u_buffer_ || u_buffer_->Size() != u_buf_size) {
            u_buffer_ = Data::Make(u_buf, u_buf_size);
            need_create_texture_ = true;
        }
        if (!v_buffer_ || v_buffer_->Size() != v_buf_size) {
            v_buffer_ = Data::Make(v_buf, v_buf_size);
            need_create_texture_ = true;
        }

        if (tex_width_ != width || tex_height_ != height) {
            need_create_texture_ = true;
        }
        memcpy(y_buffer_->DataAddr(), y_buf, y_buf_size);
        memcpy(u_buffer_->DataAddr(), u_buf, u_buf_size);
        memcpy(v_buffer_->DataAddr(), v_buf, v_buf_size);

        tex_width_ = width;
        tex_height_ = height;

        this->OnUpdate();
    }
    
}
