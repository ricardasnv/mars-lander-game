// Ricardas Navickas 2020
#include "model.h"
#include <iostream>

Model::Model() {}

Model::Model(Mesh* m, GLuint mode) {
	set_mesh(m, mode);
}

Model::Model(const Model& b) {
	mesh = new Mesh;
	*mesh = *b.mesh; // copy mesh
	draw_mode = b.draw_mode;
	set_mesh(mesh, draw_mode);
}

Model::~Model() {
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &element_buffer);
	glDeleteVertexArrays(1, &vertex_array);
}

void Model::set_mesh(Mesh* m, GLuint mode) {
	mesh = m;
	draw_mode = mode;

	// Set up VBO, VAO and EBO
	glGenVertexArrays(1, &vertex_array);
	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &element_buffer);

	glBindVertexArray(vertex_array);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->vertex_data.size(), mesh->vertex_data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->indices.size(), mesh->indices.data(), GL_STATIC_DRAW);

	// Vertex coords
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_LEN * sizeof(float), (void*)(VERTEX_COORD_OFFSET * sizeof(float)));
	glEnableVertexAttribArray(0);
	// Normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_LEN * sizeof(float), (void*)(VERTEX_NORMAL_OFFSET * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Colors
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VERTEX_DATA_LEN * sizeof(float), (void*)(VERTEX_COLOR_OFFSET * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Model::reload_mesh() {
	// Free current VBO, VAO, EBO
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &element_buffer);
	glDeleteVertexArrays(1, &vertex_array);

	set_mesh(mesh, draw_mode);
}

void Model::draw_wire() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glDrawElements(draw_mode, mesh->indices.size(), GL_UNSIGNED_INT, 0);
}

void Model::draw_solid() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	glDrawElements(draw_mode, mesh->indices.size(), GL_UNSIGNED_INT, 0);
}

