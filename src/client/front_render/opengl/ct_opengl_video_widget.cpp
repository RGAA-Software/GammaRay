#include <QTimer>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QWheelEvent>
#include <QString>
#include <QMimeData>
#include <QApplication>
#include <thread>
#include "ct_shader_program.h"
#include "ct_opengl_video_widget.h"
#include "ct_video_widget_shaders.h"
#include "tc_common_new/time_util.h"
#include "tc_common_new/log.h"
#include "tc_common_new/data.h"
#include "front_render/opengl/ct_director.h"
#include "ct_sprite.h"
#include "ct_settings.h"
#include "tc_common_new/image.h"
#include "tc_common_new/file.h"

namespace tc
{

	OpenGLVideoWidget::OpenGLVideoWidget(const std::shared_ptr<ClientContext>& ctx, const std::shared_ptr<ThunderSdk>& sdk, int dup_idx, RawImageFormat format, QWidget* parent)
		: QOpenGLWidget(parent), VideoWidget(ctx, sdk, dup_idx) {

        TimeDuration dr("OpenGLVideoWidget");
        context_ = ctx;
        raw_image_format_ = format;
        settings_ = Settings::Instance();

		setFocusPolicy(Qt::StrongFocus);
		setMouseTracking(true);
	}

	OpenGLVideoWidget::~OpenGLVideoWidget() {

	}

	void OpenGLVideoWidget::initializeGL() {

#if 0
		QSurfaceFormat format = QSurfaceFormat::defaultFormat();
		// 设置V-Sync开启
		format.setSwapInterval(1);  // 1表示启用V-Sync，0表示禁用
		QSurfaceFormat::setDefaultFormat(format);
#endif
		vao_obj_.create();
		vao_obj_.bind();
		QOpenGLFunctions* f = context()->functions();
		f->initializeOpenGLFunctions();
		gl_func_ = f;
		LOGI("initializeOpenGLFunctions, gl_func: 0x{:p}", (void*)gl_func_);

        TimeDuration duration("initializeGL");

		//auto functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
		//functions->initializeOpenGLFunctions();

        director_ = Director::Make(gl_func_);

		auto vertex_shader = kMainVertexShader;
		char* fragment_shader = nullptr;

		if (raw_image_format_ == RawImageFormat::kRawImageNV12) {
			fragment_shader = const_cast<char*>(kNV12FragmentShader);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageRGB || raw_image_format_ == RawImageFormat::kRawImageRGBA) {
			fragment_shader = const_cast<char*>(kRGBFragmentShader);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI420) {
			fragment_shader = const_cast<char*>(kI420FragmentShader);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI444) {
			fragment_shader = const_cast<char*>(kI444FragmentShader);
		}
		if (!fragment_shader) {
			return;
		}
		
		shader_program_ = ShaderProgram::Make(gl_func_, vertex_shader, fragment_shader);

		constexpr float vertices[] = {
			-1.0f, -1.0f, 0.0f, 1.0f, 0, 0,  0, 0,
			1.0f, -1.0f, 0.0f, 0, 1.0f, 0,  1, 0,
			1.0f,  1.0f, 0.0f, 0, 0, 1.0f,  1, 1,
			-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1
		};

		constexpr int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		//gl_func_->glGenVertexArrays(1, &vao_);
		//gl_func_->glBindVertexArray(vao_);

		gl_func_->glGenBuffers(1, &vbo_);
		gl_func_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		gl_func_->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		int posLoc = shader_program_->GetAttribLocation("aPos");
		gl_func_->glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, 8 * 4, 0);
		gl_func_->glEnableVertexAttribArray(posLoc);

		int colorLoc = shader_program_->GetAttribLocation("aColor");
		gl_func_->glVertexAttribPointer(colorLoc, 3, GL_FLOAT, false, 8 * 4, (void*)(3 * 4));
		gl_func_->glEnableVertexAttribArray(colorLoc);

		int texLoc = shader_program_->GetAttribLocation("aTex");
		gl_func_->glVertexAttribPointer(texLoc, 2, GL_FLOAT, false, 8 * 4, (void*)(6 * 4));
		gl_func_->glEnableVertexAttribArray(texLoc);

		gl_func_->glGenBuffers(1, &ibo_);
		gl_func_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
		gl_func_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		//glBindVertexArray(0);
		vao_obj_.release();

        // cursor
        cursor_ = std::make_shared<Sprite>(director_);

        // logo
        if (settings_->display_logo_) {
            logo_ = std::make_shared<Sprite>(director_);
            auto data = File::OpenForReadB(":/resources/image/logo_text.png")->ReadAll();
            auto image = Image::MakeByCompressedImage(data);
            auto raw_image = RawImage::MakeRGBA(image->data->DataAddr(), image->data->Size(), image->width, image->height);
            logo_->UpdateImage(raw_image);
            logo_->ForceImageSize(image->width, image->height);
        }
	}

	void OpenGLVideoWidget::RefreshImage(const std::shared_ptr<RawImage>& image) {

		//std::string file_name = QString(".\\yuv\\debug.yuv444").toStdString();
		//image->AppendYUV444ToFile(file_name);

		VideoWidget::RefreshImage(image);
        SetDisplayImageFormat(image->img_format);
	}

	void OpenGLVideoWidget::resizeEvent(QResizeEvent* event) {
		QOpenGLWidget::resizeEvent(event);
        //if (event->size().width() <= 0 || event->size().height() <= 0) {
        //    return;
        //}
        //gl_func_->glViewport(0, 0, event->size().width(), event->size().height());
		//LOGI("resize event, {}x{}", event->size().width(), event->size().height());
	}

	void OpenGLVideoWidget::paintGL() {
        std::lock_guard<std::mutex> guard(buf_mtx_);
		if (!shader_program_) {
			return;
		}

		gl_func_->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

		gl_func_->glDisable(GL_SCISSOR_TEST);
		gl_func_->glDisable(GL_DEPTH_TEST);

		gl_func_->glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
		gl_func_->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		vao_obj_.bind();
		//glBindVertexArray(vao_);
		shader_program_->Active();
		shader_program_->SetUniform1i("haveImage", 1);

		gl_func_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		gl_func_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
		gl_func_->glPixelStorei(GL_PACK_ALIGNMENT, 1);
		gl_func_->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (raw_image_format_ == RawImageFormat::kRawImageRGBA || raw_image_format_ == RawImageFormat::kRawImageRGB) {
			if (rgb_buffer_ && need_create_texture_) {
				need_create_texture_ = false;
				InitRGBATexture();
			}
			if (rgb_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE0);
				gl_func_->glBindTexture(GL_TEXTURE_2D, rgb_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RGB, GL_UNSIGNED_BYTE, rgb_buffer_->DataAddr());
			}
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI420) {
			if (y_buffer_ && u_buffer_ && v_buffer_ && need_create_texture_) {
				need_create_texture_ = false;
				InitI420Texture();
			}

			if (y_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE0);
				gl_func_->glBindTexture(GL_TEXTURE_2D, y_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RED, GL_UNSIGNED_BYTE, y_buffer_->CStr());
			}
			if (u_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE1);
				gl_func_->glBindTexture(GL_TEXTURE_2D, u_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_/2, tex_height_/2, GL_RED, GL_UNSIGNED_BYTE, u_buffer_->CStr());
			}
			if (v_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE2);
				gl_func_->glBindTexture(GL_TEXTURE_2D, v_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_ / 2, tex_height_ / 2, GL_RED, GL_UNSIGNED_BYTE, v_buffer_->CStr());
			}
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI444) {
			if (y_buffer_ && u_buffer_ && v_buffer_ && need_create_texture_) {
				need_create_texture_ = false;
				InitI444Texture();
			}

			if (y_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE0);
				gl_func_->glBindTexture(GL_TEXTURE_2D, y_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RED, GL_UNSIGNED_BYTE, y_buffer_->CStr());
			}
			if (u_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE1);
				gl_func_->glBindTexture(GL_TEXTURE_2D, u_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RED, GL_UNSIGNED_BYTE, u_buffer_->CStr());
			}
			if (v_buffer_) {
				gl_func_->glActiveTexture(GL_TEXTURE2);
				gl_func_->glBindTexture(GL_TEXTURE_2D, v_texture_id_);
				gl_func_->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_width_, tex_height_, GL_RED, GL_UNSIGNED_BYTE, v_buffer_->CStr());
			}
		}

		if (raw_image_format_ == RawImageFormat::kRawImageRGB || raw_image_format_ == RawImageFormat::kRawImageRGBA) {
			shader_program_->SetUniform1i("image1", 0);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageNV12) {
			shader_program_->SetUniform1i("image1", 0);
			shader_program_->SetUniform1i("image2", 1);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI420) {
			shader_program_->SetUniform1i("imageY", 0);
			shader_program_->SetUniform1i("imageU", 1);
			shader_program_->SetUniform1i("imageV", 2);
		}
		else if (raw_image_format_ == RawImageFormat::kRawImageI444) {
			shader_program_->SetUniform1i("imageY", 0);
			shader_program_->SetUniform1i("imageU", 1);
			shader_program_->SetUniform1i("imageV", 2);
		}

		model_ = glm::mat4(1.0f);
		shader_program_->SetUniformMatrix("model", model_);
        shader_program_->SetUniformMatrix("view", director_->GetView());
        shader_program_->SetUniformMatrix("projection", director_->GetProjection());

		gl_func_->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		vao_obj_.release();
		//glBindVertexArray(0);
		shader_program_->Release();

        if (cursor_) {
            cursor_->Render(0);
        }
        if (logo_) {
            logo_->Render(0);
        }
		render_fps_ += 1;
		auto current_time = TimeUtil::GetCurrentTimestamp();
		if (current_time - last_update_fps_time_ > 1000) {
			render_fps_ = 0;
			last_update_fps_time_ = current_time;
		}
	}

	void OpenGLVideoWidget::InitRGBATexture() {
		gl_func_->glGenTextures(1, &rgb_texture_id_);
		gl_func_->glBindTexture(GL_TEXTURE_2D, rgb_texture_id_);
		gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl_func_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	void OpenGLVideoWidget::InitI420Texture() {
		auto create_luminance_texture = [this](GLuint& tex_id, int width, int height, bool is_uv) {
			gl_func_->glGenTextures(1, &tex_id);
			gl_func_->glBindTexture(GL_TEXTURE_2D, tex_id);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl_func_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, is_uv ? width / 2 : width, is_uv ? height / 2 : height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			gl_func_->glBindTexture(GL_TEXTURE_2D, 0);
		};
		create_luminance_texture(y_texture_id_, tex_width_, tex_height_, false);
		create_luminance_texture(u_texture_id_, tex_width_, tex_height_, true);
		create_luminance_texture(v_texture_id_, tex_width_, tex_height_, true);

        LOGI("Init I420 texture : {} x {} ", tex_width_, tex_height_);
	}

	void OpenGLVideoWidget::InitI444Texture() {
		auto create_luminance_texture = [this](GLuint& tex_id, int width, int height) {
			gl_func_->glGenTextures(1, &tex_id);
			gl_func_->glBindTexture(GL_TEXTURE_2D, tex_id);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			gl_func_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			gl_func_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			gl_func_->glBindTexture(GL_TEXTURE_2D, 0);
		};
		create_luminance_texture(y_texture_id_, tex_width_, tex_height_);
		create_luminance_texture(u_texture_id_, tex_width_, tex_height_);
		create_luminance_texture(v_texture_id_, tex_width_, tex_height_);

		LOGI("Init I444 texture : {} x {} ", tex_width_, tex_height_);
	}

    void OpenGLVideoWidget::OnUpdate() {
        QMetaObject::invokeMethod(this, [this]() {
            this->repaint();
        });
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

	void OpenGLVideoWidget::resizeGL(int width, int height) {
		VideoWidget::OnWidgetResize(width, height);

		gl_func_->glViewport(0, 0, width, height);

		director_->Init(width, height);

		LOGI("resize GL: {} x {}", width, height);
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
		VideoWidget::OnMouseMoveEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mousePressEvent(QMouseEvent* e) {
		QOpenGLWidget::mousePressEvent(e);
		VideoWidget::OnMousePressEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mouseReleaseEvent(QMouseEvent* e) {
		QOpenGLWidget::mouseReleaseEvent(e);
		VideoWidget::OnMouseReleaseEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::mouseDoubleClickEvent(QMouseEvent* e) {
		QOpenGLWidget::mouseDoubleClickEvent(e);
        VideoWidget::OnMouseDoubleClickEvent(e);
	}

	void OpenGLVideoWidget::wheelEvent(QWheelEvent* e) {
		QOpenGLWidget::wheelEvent(e);
		VideoWidget::OnWheelEvent(e, QWidget::width(), QWidget::height());
	}

	void OpenGLVideoWidget::keyPressEvent(QKeyEvent* e) {
		QOpenGLWidget::keyPressEvent(e);
		VideoWidget::OnKeyPressEvent(e);
	}

	void OpenGLVideoWidget::keyReleaseEvent(QKeyEvent* e) {
		QOpenGLWidget::keyReleaseEvent(e);
		VideoWidget::OnKeyReleaseEvent(e);
	}

    void OpenGLVideoWidget::Exit() {
        makeCurrent();
        //glDeleteVertexArrays(1, &vao_);
        //glDeleteBuffers(1, &vbo_);
		vao_obj_.destroy();
        if (shader_program_) {
            shader_program_->Release();
        }

        if (y_texture_id_) {
            gl_func_->glDeleteTextures(1, &y_texture_id_);
        }
        if (uv_texture_id_) {
            gl_func_->glDeleteTextures(1, &uv_texture_id_);
        }
        if (u_texture_id_) {
            gl_func_->glDeleteTextures(1, &u_texture_id_);
        }
        if (v_texture_id_) {
            gl_func_->glDeleteTextures(1, &v_texture_id_);
        }
        if (rgb_texture_id_) {
            gl_func_->glDeleteTextures(1, &rgb_texture_id_);
        }
        doneCurrent();
    }

    QWidget* OpenGLVideoWidget::AsWidget() {
        return dynamic_cast<QWidget*>(this);
    }

}
