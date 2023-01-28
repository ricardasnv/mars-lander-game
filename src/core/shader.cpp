// Ricardas Navickas 2020
#include "shader.h"
#include "fileio.h"
#include "error.h"
#include "graphics.h"
#include "glm/gtc/type_ptr.hpp"

Shader::Shader(const char* vertex_src_path, const char* fragment_src_path) {
	int success;
	char info[512];

	char* vsrc = read_file(vertex_src_path);
	char* fsrc = read_file(fragment_src_path);

	// Compile vertex shader
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vsrc, NULL);
	glCompileShader(vertex_shader);

	// Check for compile errors
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, info);
		error("Shader::Shader()", info);
	}

	// Compile fragment shader
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fsrc, NULL);
	glCompileShader(fragment_shader);

	// Check for compile errors
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, info);
		error("Shader::Shader()", info);
	}

	// Link program
	id = glCreateProgram();
	glAttachShader(id, vertex_shader);
	glAttachShader(id, fragment_shader);
	glLinkProgram(id);

	// Check for linking errors
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(id, 512, NULL, info);
		error("Shader::Shader()", info);
	}

	// Clean up
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

Shader::~Shader() {
	glDeleteProgram(id);
}

void Shader::use() {
	glUseProgram(id);
}

// Set uniform variables
void Shader::setb(std::string name, bool val) {
	glUniform1i(glGetUniformLocation(id, name.c_str()), (int)val);
}

void Shader::seti(std::string name, int val) {
	glUniform1i(glGetUniformLocation(id, name.c_str()), val);
}

void Shader::setf(std::string name, float val) {
	glUniform1f(glGetUniformLocation(id, name.c_str()), val);
}

void Shader::set3f(std::string name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::set4f(std::string name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

void Shader::set3fv(std::string name, unsigned int n, std::vector<glm::vec3> v) {
	glUniform3fv(glGetUniformLocation(id, name.c_str()), n, glm::value_ptr(v[0]));
}

void Shader::setmat4(std::string name, glm::mat4 mat) {
	glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

