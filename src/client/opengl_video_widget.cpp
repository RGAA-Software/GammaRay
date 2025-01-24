#include <QDebug>
#include <QTimer>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QString>
#include <QMimeData>
#include <QApplication>
#include <QWindow>
#include <QScreen>

#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

#include "shader_program.h"
#include "opengl_video_widget.h"
#include "video_widget_shaders.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "director.h"
#include "sprite.h"
#include "tc_common_new/image.h"
#include "tc_common_new/file.h"

namespace tc
{

	OpenGLVideoWidget::OpenGLVideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx, RawImageFormat format, QWidget* parent)
		: QOpenGLWidget(parent), VideoWidgetEvent(ctx, sdk, dup_idx) {

        context_ = ctx;
        raw_image_format_ = format;
//		statistics = Statistics::Instance();

		setFocusPolicy(Qt::StrongFocus);
		setMouseTracking(true);

        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            this->update();
        });
        timer->start(17);
	}

	OpenGLVideoWidget::~OpenGLVideoWidget() {

	}

	void OpenGLVideoWidget::initializeGL() {
		initializeOpenGLFunctions();

		auto functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
		functions->initializeOpenGLFunctions();

        director_ = Director::Make(functions);

		auto vertex_shader = kMainVertexShader;
		char* fragment_shader = nullptr;

		if (raw_image_format_ == RawImageFormat::kNV12) {
			fragment_shader = const_cast<char*>(kNV12FragmentShader);
		}
		else if (raw_image_format_ == RawImageFormat::kRGB || raw_image_format_ == RawImageFormat::kRGBA) {
			fragment_shader = const_cast<char*>(kRGBFragmentShader);
		}
		else if (raw_image_format_ == RawImageFormat::kI420) {
			fragment_shader = const_cast<char*>(kI420FragmentShader);
		}
		if (!fragment_shader) {
			return;
		}
		
		shader_program_ = ShaderProgram::Make(functions, vertex_shader, fragment_shader);

		float vertices[] = {
			-1.0f, -1.0f, 0.0f, 1.0f, 0, 0,  0, 0,
			1.0f, -1.0f, 0.0f, 0, 1.0f, 0,  1, 0,
			1.0f,  1.0f, 0.0f, 0, 0, 1.0f,  1, 1,
			-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1
		};

		int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		glGenVertexArrays(1, &vao_);
		glBindVertexArray(vao_);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		int posLoc = shader_program_->GetAttribLocation("aPos");
		glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, 8 * 4, 0);
		glEnableVertexAttribArray(posLoc);

		int colorLoc = shader_program_->GetAttribLocation("aColor");
		glVertexAttribPointer(colorLoc, 3, GL_FLOAT, false, 8 * 4, (void*)(3 * 4));
		glEnableVertexAttribArray(colorLoc);

		int texLoc = shader_program_->GetAttribLocation("aTex");
		glVertexAttribPointer(texLoc, 2, GL_FLOAT, false, 8 * 4, (void*)(6 * 4));
		glEnableVertexAttribArray(texLoc);

		glGenBuffers(1, &ibo_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glBindVertexArray(0);

        // cursor
        cursor_ = std::make_shared<Sprite>(director_);

        // logo
        logo_ = std::make_shared<Sprite>(director_);
        auto data = File::OpenForReadB(":/resources/image/logo_text.png")->ReadAll();
        auto image = Image::MakeByCompressedImage(data);
        auto raw_image = RawImage::MakeRGBA(image->data->DataAddr(), image->data->Size(), image->width, image->height);
        logo_->UpdateImage(raw_image);
        logo_->ForceImageSize(image->width, image->height);

	}

	void OpenGLVideoWidget::RefreshRGBImage(const std::shared_ptr<RawImage>& image) {
		assert(image->Size() == image->img_width * image->img_height * image->img_ch);
		RefreshRGBBuffer(image->Data(), image->img_width, image->img_height, image->img_ch);
	}

	void OpenGLVideoWidget::RefreshRGBBuffer(const char* buf, int width, int height, int channel) {
		std::lock_guard<std::mutex> guard(buf_mtx_);
		int size = width * height * channel;
		if (!rgb_buffer_) {
			rgb_buffer_ = (char*)malloc(size);
		}
		if (tex_width_ != width || tex_height_ != height) {
			need_create_texture_ = true;
		}
		memcpy(rgb_buffer_, buf, size);
		tex_width_ = width;
		tex_height_ = height;
		tex_channel_ = channel;
	}

	void OpenGLVideoWidget::RefreshI420Image(const std::shared_ptr<RawImage>& image) {
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

	void OpenGLVideoWidget::RefreshI420Buffer(const char* y_buf, int y_buf_size,
		const char* u_buf, int u_buf_size,
		const char* v_buf, int v_buf_size,
		int width, int height) {
        auto target_y_size = width * height;
        auto target_u_size = width/2 * height/2;
		if (!y_buffer_ || y_buffer_->Size() != y_buf_size) {
			y_buffer_ = Data::Make(y_buf, y_buf_size);
		}
		if (!u_buffer_ || u_buffer_->Size() != u_buf_size) {
			u_buffer_ = Data::Make(u_buf, u_buf_size);
		}
		if (!v_buffer_ || v_buffer_->Size() != v_buf_size) {
			v_buffer_ = Data::Make(v_buf, v_buf_size);
		}
	
		if (tex_width_ != width || tex_height_ != height) {
			need_create_texture_ = true;
		}
		memcpy(y_buffer_->DataAddr(), y_buf, y_buf_size);
		memcpy(u_buffer_->DataAddr(), u_buf, u_buf_size);
		memcpy(v_buffer_->DataAddr(), v_buf, v_buf_size);
		
		tex_width_ = width;
		tex_height_ = height;
	}

	void OpenGLVideoWidget::resizeEvent(QResizeEvent* event) {
		QOpenGLWidget::resizeEvent(event);
        if (event->size().width() <= 0 || event->size().height() <= 0) {
            return;
        }
        glViewport(0, 0, event->size().width(), event->size().height());
	}

	void OpenGLVideoWidget::paintGL() {
        std::lock_guard<std::mutex> guard(buf_mtx_);
		if (!shader_program_) {
			return;
		}

		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(vao_);
		shader_program_->Active();

		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (raw_image_format_ == RawImageFormat::kRGBA || raw_image_format_ == RawImageFormat::kRGB) {
			if (rgb_buffer_ && need_create_texture_) {
				need_create_texture_ = false;
				InitRGBATexture();
			}
			if (rgb_buffer_) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rgb_texture_id_);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer_);
			}
		}
		else if (raw_image_format_ == RawImageFormat::kI420) {
			if (y_buffer_ && u_buffer_ && v_buffer_ && need_create_texture_) {
				need_create_texture_ = false;
				InitI420Texture();
			}

			if (y_buffer_) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, y_texture_id_);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RED, GL_UNSIGNED_BYTE, y_buffer_->CStr());
			}
			if (u_buffer_) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, u_texture_id_);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_/2, tex_height_/2, GL_RED, GL_UNSIGNED_BYTE, u_buffer_->CStr());
			}
			if (v_buffer_) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, v_texture_id_);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_ / 2, tex_height_ / 2, GL_RED, GL_UNSIGNED_BYTE, v_buffer_->CStr());
			}
		}

		if (raw_image_format_ == RawImageFormat::kRGB || raw_image_format_ == RawImageFormat::kRGBA) {
			shader_program_->SetUniform1i("image1", 0);
		}
		else if (raw_image_format_ == RawImageFormat::kNV12) {
			shader_program_->SetUniform1i("image1", 0);
			shader_program_->SetUniform1i("image2", 1);
		}
		else if (raw_image_format_ == RawImageFormat::kI420) {
			shader_program_->SetUniform1i("imageY", 0);
			shader_program_->SetUniform1i("imageU", 1);
			shader_program_->SetUniform1i("imageV", 2);
		}

		model_ = glm::mat4(1.0f);
		shader_program_->SetUniformMatrix("model", model_);
        shader_program_->SetUniformMatrix("view", director_->GetView());
        shader_program_->SetUniformMatrix("projection", director_->GetProjection());

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		shader_program_->Release();

        if (cursor_) {
            cursor_->Render(0);
        }
        if (logo_) {
            logo_->Render(0);
        }
		render_fps_ += 1;
		auto current_time = TimeExt::GetCurrentTimestamp();
		if (current_time - last_update_fps_time_ > 1000) {
			//statistics->render_fps = render_fps;
			render_fps_ = 0;
			last_update_fps_time_ = current_time;
		}
	}

	void OpenGLVideoWidget::InitRGBATexture() {
		glGenTextures(1, &rgb_texture_id_);
		glBindTexture(GL_TEXTURE_2D, rgb_texture_id_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	void OpenGLVideoWidget::InitI420Texture() {
		auto create_luminance_texture = [this](GLuint& tex_id, int width, int height, bool is_uv) {
			glGenTextures(1, &tex_id);
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, is_uv ? width / 2 : width, is_uv ? height / 2 : height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		};
		create_luminance_texture(y_texture_id_, tex_width_, tex_height_, false);
		create_luminance_texture(u_texture_id_, tex_width_, tex_height_, true);
		create_luminance_texture(v_texture_id_, tex_width_, tex_height_, true);

        LOGI("Init I420 texture : {} x {} ", tex_width_, tex_height_);
	}

    void OpenGLVideoWidget::RefreshCursor(int x, int y, int tex_left, int tex_right, int hpx, int hpy, const std::shared_ptr<RawImage>& cursor) {
        if (!cursor_ || tex_width_ == 0 || tex_height_ == 0) {
            return;
        }

        auto buf = (uint32_t *)cursor->img_buf;
        for (int row = 0; row < cursor->img_height; row++) {
            auto last_pixel = buf + row * cursor->img_width + (cursor->img_width - 1);
            *last_pixel = 0x00000000;
        }

        int cal_tex_width = tex_right - tex_left;
        int x_value_from_left = x - tex_left;
        float x_percent_from_left = x_value_from_left * 1.0f / cal_tex_width;
        float target_x = x_percent_from_left * cal_tex_width;
        float xp = target_x * 1.0f / cal_tex_width;
        float yp = y * 1.0f / tex_height_;
        cursor_->UpdateTranslationPercentWindow(xp, yp);

        LOGI("target x: {}, xp: {}", target_x, xp);

        float ratio_x = QWidget::width() * 1.0f / tex_width_;
        float ratio_y = QWidget::height() * 1.0f / tex_height_;
        float adjust_x = hpx * ratio_x;
        float adjust_y = hpy * ratio_y;

        cursor_->ForceImageSize(cursor->img_width * ratio_x, cursor->img_height * ratio_y);

        cursor_->UpdateTranslationAdjuster(-adjust_x, adjust_y);
        cursor_->UpdateImage(cursor);

        //LOGI("xp : {}, yp : {}, size: {}x{}", xp, yp, cursor->img_width, cursor->img_height);
    }

    void OpenGLVideoWidget::RefreshCapturedMonitorInfo(const CaptureMonitorInfo& mon_info) {
        cap_mon_info_ = mon_info;
    }

    int OpenGLVideoWidget::GetCapturingMonitorWidth() {
        return cap_mon_info_.Width();
    }

    int OpenGLVideoWidget::GetCapturingMonitorHeight() {
        return cap_mon_info_.Height();
    }

	void OpenGLVideoWidget::resizeGL(int width, int height) {
		VideoWidgetEvent::OnWidgetResize(width, height);
		glViewport(0, 0, width, height);

        director_->Init(width, height);
        glViewport(0, 0, width, height);
        if (cursor_) {
            cursor_->OnWindowResized(width, height);
        }
        if (logo_) {
            logo_->OnWindowResized(width, height);
            auto image = logo_->GetRawImage();
            if (image) {
                auto x = 1.0f - image->img_width*1.0f / width;
                logo_->UpdateTranslationPercentWindow(x, 0.0f);
            }
        }
	}

	void OpenGLVideoWidget::mouseMoveEvent(QMouseEvent* e) {
		QOpenGLWidget::mouseMoveEvent(e);
		VideoWidgetEvent::OnMouseMoveEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mousePressEvent(QMouseEvent* e) {
		QOpenGLWidget::mousePressEvent(e);
		VideoWidgetEvent::OnMousePressEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mouseReleaseEvent(QMouseEvent* e) {
		QOpenGLWidget::mouseReleaseEvent(e);
		VideoWidgetEvent::OnMouseReleaseEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mouseDoubleClickEvent(QMouseEvent* e) {
		QOpenGLWidget::mouseDoubleClickEvent(e);
        VideoWidgetEvent::OnMouseDoubleClickEvent(e);
	}

	void OpenGLVideoWidget::wheelEvent(QWheelEvent* e) {
		QOpenGLWidget::wheelEvent(e);
		VideoWidgetEvent::OnWheelEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::keyPressEvent(QKeyEvent* e) {
		QOpenGLWidget::keyPressEvent(e);
		VideoWidgetEvent::OnKeyPressEvent(e);
	}

	void OpenGLVideoWidget::keyReleaseEvent(QKeyEvent* e) {
		QOpenGLWidget::keyReleaseEvent(e);
		VideoWidgetEvent::OnKeyReleaseEvent(e);
	}

    void OpenGLVideoWidget::Exit() {
        makeCurrent();
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        if (shader_program_) {
            shader_program_->Release();
        }

        if (y_texture_id_) {
            glDeleteTextures(1, &y_texture_id_);
        }
        if (uv_texture_id_) {
            glDeleteTextures(1, &uv_texture_id_);
        }
        if (u_texture_id_) {
            glDeleteTextures(1, &u_texture_id_);
        }
        if (v_texture_id_) {
            glDeleteTextures(1, &v_texture_id_);
        }
        if (rgb_texture_id_) {
            glDeleteTextures(1, &rgb_texture_id_);
        }
        doneCurrent();
    }

}
