// Ricardas Navickas 2020
#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "GL/glew.h"
#include "GL/gl.h"
#include "mesh.h"

class Model {
public:
	Model();
	Model(Mesh* m, GLuint mode);
	Model(const Model&);
	~Model();

	void set_mesh(Mesh* m, GLuint mode);
	void reload_mesh(); // Updates model if mesh has changed

	void draw_wire();
	void draw_solid();

	Mesh* mesh;
	GLuint draw_mode;

private:
	// VBO, VAO and EBO
	GLuint vertex_buffer;
	GLuint vertex_array;
	GLuint element_buffer;
};

#endif
