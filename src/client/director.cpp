#include "director.h"

namespace tc
{

	std::shared_ptr<Director> Director::Make(GLFuncs* funcs) {
		return std::make_shared<Director>(funcs);
	}

	Director::Director(GLFuncs* funcs) {
		this->funcs = funcs;
	}

	Director::~Director() {
		
	}

	void Director::Init(int width, int height) {
		this->width = width;
		this->height = height;

		float near_plane = 0.0f, far_plane = 100.0f;
		
		projection = glm::ortho(0.0f, this->width, 0.0f, this->height, near_plane, far_plane);
		view = glm::mat4(1.0f);
	}

	glm::mat4 Director::GetProjection() {
		return projection;
	}

	glm::mat4 Director::GetView() {
		return view;
	}

	GLFuncs* Director::Funcs() {
		return funcs;
	}

}