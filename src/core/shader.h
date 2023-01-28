// Ricardas Navickas 2020
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>

#include "glm/glm.hpp"

class Shader {
public:
	Shader(const char* vertex_src_path, const char* fragment_src_path);
	~Shader();

	unsigned int id;

	// Calls glUseProgram(id)
	void use();

	// Set uniform variables
	void setb(std::string name, bool val);
	void seti(std::string name, int val);
	void setf(std::string name, float val);
	void set3f(std::string name, float x, float y, float z);
	void set4f(std::string name, float x, float y, float z, float w);

	void set3fv(std::string name, unsigned int n, std::vector<glm::vec3> v);
	void setmat4(std::string name, glm::mat4 mat);
};

#endif
