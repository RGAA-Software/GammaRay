#include "sprite.h"

#include <iostream>

#include "director.h"
#include "tc_client_sdk_new/gl/raw_image.h"
#include "shader_program.h"
#include "video_widget_shaders.h"

namespace tc
{

	Sprite::Sprite(const std::shared_ptr<Director>& director) : Renderer(director) {
		Init();
	}

	Sprite::~Sprite() {
	
	}

	void Sprite::Init() {
		functions->glGenVertexArrays(1, &render_vao);
		functions->glBindVertexArray(render_vao);

		shader_program = ShaderProgram::Make(functions, kOperationVertexShader, kRGBAFragmentShader);

		float vertices[] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0, 0,  0, 0,
			80.0f, 0.0f, 0.0f, 0, 1.0f, 0,  1, 0,
			80.0f,  80.0f, 0.0f, 0, 0, 1.0f,  1, 1,
			0.0f, 80.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1
		};

		int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		functions->glGenBuffers(1, &vbo);
		functions->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		functions->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		int posLoc = functions->glGetAttribLocation(shader_program->GetProgramId(), "aPos");
		functions->glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, 8 * 4, 0);
		functions->glEnableVertexAttribArray(posLoc);

		int colorLoc = functions->glGetAttribLocation(shader_program->GetProgramId(), "aColor");
		functions->glVertexAttribPointer(colorLoc, 3, GL_FLOAT, false, 8 * 4, (void*)(3 * 4));
		functions->glEnableVertexAttribArray(colorLoc);

		int texLoc = functions->glGetAttribLocation(shader_program->GetProgramId(), "aTex");
		functions->glVertexAttribPointer(texLoc, 2, GL_FLOAT, false, 8 * 4, (void*)(6 * 4));
		functions->glEnableVertexAttribArray(texLoc);

		GLuint ibo;
		functions->glGenBuffers(1, &ibo);
		functions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		functions->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		functions->glGenTextures(1, &texture_id);
		functions->glBindTexture(GL_TEXTURE_2D, texture_id);
		functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//functions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		//functions->glGenerateMipmap(GL_TEXTURE_2D);

		functions->glBindVertexArray(0);
	}

	void Sprite::UpdateImage(const std::shared_ptr<RawImage>& image) {
		std::lock_guard<std::mutex> guard(buf_mtx);
		this->image_ = image;
	}

	void Sprite::ForceImageSize(int width, int height) {
		force_width = width;
		force_height = height;
	}

	void Sprite::UpdateTranslation(int x, int y) {
		trans_x = x;
		trans_y = y;
	}

	void Sprite::UpdateTranslationPercentWindow(float x, float y) {
		trans_xf = x;
		trans_yf = y;
	}

	void Sprite::UpdateTranslationAdjuster(float x, float y) {
		adjuster_x = x;
		adjuster_y = y;
	}

	void Sprite::Render(float delta) {
		{
			std::lock_guard<std::mutex> guard(buf_mtx);
			if (!image_) {
				return;
			}
		}

		functions->glBindVertexArray(render_vao);
		shader_program->Active();

		functions->glEnable(GL_BLEND);
		functions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(window_width * trans_xf + adjuster_x, window_height * (1.0f - trans_yf) + adjuster_y, 0));
		shader_program->SetUniformMatrix("model", model);
		shader_program->SetUniformMatrix("view", director->GetView());
		shader_program->SetUniformMatrix("projection", director->GetProjection());

		int target_width = force_width > 0 ? force_width : image_->img_width;
		int target_height = force_height > 0 ? force_height : image_->img_height;

		float vertices[] = {
			0.0f, 0.0f, 0.0f, 1.0f, 0, 0,  0, 0,
			target_width * 1.0f, 0.0f, 0.0f, 0, 1.0f, 0,  1, 0,
			target_width * 1.0f, -target_height*1.0f, 0.0f, 0, 0, 1.0f,  1, 1,
			0.0f, -target_height *1.0f, 0.0f, 1.0f, 1.0f, 0, 0, 1
		};

		functions->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		functions->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		functions->glActiveTexture(GL_TEXTURE0);
		functions->glBindTexture(GL_TEXTURE_2D, texture_id);
		functions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_->img_width, image_->img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_->Data());
		functions->glUniform1i(functions->glGetUniformLocation(shader_program->GetProgramId(), "image1"), 0);
		functions->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		functions->glBindVertexArray(0);

        functions->glDisable(GL_BLEND);
	}

    std::shared_ptr<RawImage> Sprite::GetRawImage() {
        return image_;
    }

}