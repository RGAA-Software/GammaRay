#pragma once

#include <mutex>
#include <memory>
#include <string>

#include <QOpenGLFunctions_3_3_Core>

#include "renderer.h"

namespace tc
{
	
	class Director;
	class RawImage;
	class ShaderProgram;

	class Sprite : public Renderer {
	public:

        explicit Sprite(const std::shared_ptr<Director>& director);
		~Sprite() override;

		void Init();
		void Render(float delta) override;

		void UpdateImage(const std::shared_ptr<RawImage>& image);
		void UpdateTranslation(int x, int y);
		void UpdateTranslationPercentWindow(float x, float y);
		void UpdateTranslationAdjuster(float x, float y);
		void ForceImageSize(int width, int height);
        std::shared_ptr<RawImage> GetRawImage();

	private:
		std::shared_ptr<RawImage> image_ = nullptr;
	
		GLuint texture_id = 0;
		GLuint vbo;
		std::mutex buf_mtx;

		int force_width = 0;
		int force_height = 0;

		int trans_x = 0;
		int trans_y = 0;

		float trans_xf = 0;
		float trans_yf = 0;

		float adjuster_x = 0;
		float adjuster_y = 0;
	};

}