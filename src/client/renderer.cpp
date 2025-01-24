#include "renderer.h"

#include "director.h"

namespace tc
{

	Renderer::Renderer(const std::shared_ptr<Director>& director) {
		this->director = director;
		this->functions = director->Funcs();
	}

	Renderer::~Renderer() {
	
	}

	void Renderer::Render(float delta) {

	}

	void Renderer::SetTargetSize(int width, int height) {
		this->width = width;
		this->height = height;
	}

	void Renderer::OnWindowResized(int width, int height) {
		window_width = width;
		window_height = height;
	}

}