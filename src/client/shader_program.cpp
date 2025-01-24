#include "shader_program.h"

#include <iostream>

namespace tc
{

	std::shared_ptr<ShaderProgram> ShaderProgram::Make(QOpenGLFunctions_3_3_Core* fk, const std::string& vertex, const std::string& fragment) {
		return std::make_shared<ShaderProgram>(fk, vertex, fragment);
	}

	ShaderProgram::ShaderProgram(QOpenGLFunctions_3_3_Core* fks, const std::string& vertex, const std::string& fragment) {
		this->fk_ = fks;

		GLuint vertex_shader = fk_->glCreateShader(GL_VERTEX_SHADER);
		char const* vertex_source = vertex.c_str();
		fk_->glShaderSource(vertex_shader, 1, &vertex_source, NULL);
		fk_->glCompileShader(vertex_shader);
		CheckShaderError(vertex_shader, "vertex");

		GLuint fragment_shader = fk_->glCreateShader(GL_FRAGMENT_SHADER);
		char const* fragment_source = fragment.c_str();
		fk_->glShaderSource(fragment_shader, 1, &fragment_source, NULL);

		fk_->glCompileShader(fragment_shader);
		CheckShaderError(fragment_shader, "fragment");

		program_id_ = fk_->glCreateProgram();

		fk_->glAttachShader(program_id_, vertex_shader);
		fk_->glAttachShader(program_id_, fragment_shader);

		fk_->glLinkProgram(program_id_);
		CheckShaderError(program_id_, "program");
	}

	ShaderProgram::~ShaderProgram() {

	}

	void ShaderProgram::CheckShaderError(GLuint id, const std::string& type) {
		int check_flag;
		char check_info[1024];
		if (type != "program") {
			fk_->glGetShaderiv(id, GL_COMPILE_STATUS, &check_flag);
			if (!check_flag) {
				fk_->glGetShaderInfoLog(id, 1024, NULL, check_info);
				std::cout << type << " error:" << check_info << std::endl;;
			}
		}
		else {
			fk_->glGetShaderiv(id, GL_LINK_STATUS, &check_flag);
			if (!check_flag) {
				fk_->glGetProgramInfoLog(id, 1024, NULL, check_info);
				std::cout << type << " error:" << check_info << std::endl;
			}
		}
	}

	int ShaderProgram::GetProgramId() {
		return program_id_;
	}

	void ShaderProgram::Active() {
		fk_->glUseProgram(program_id_);
	}
}