#pragma once

#include <memory>
#include <QOpenGLFunctions_3_3_Core>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace tc
{

	class Director;
	class ShaderProgram;

	class Renderer {
	public:
		explicit Renderer(const std::shared_ptr<Director>& director);
		virtual ~Renderer();

		virtual void Render(float delta);
		void OnWindowResized(int width, int height);
		void SetTargetSize(int width, int height);

	protected:
		QOpenGLFunctions_3_3_Core* functions = nullptr;
		std::shared_ptr<Director> director = nullptr;
		std::shared_ptr<ShaderProgram> shader_program = nullptr;
		glm::mat4 model{};
		GLuint render_vao = 0;

		int width = 0;
		int height = 0;

		int window_width = 0;
		int window_height = 0;

	private:
		


	};

}