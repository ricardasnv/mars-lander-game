// Ricardas Navickas 2020
#include "mesh.h"
#include "error.h"
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>

Mesh make_mesh(std::vector<float> vertex_data, std::vector<unsigned int> indices) {
	Mesh mesh;
	mesh.vertex_data = vertex_data;
	mesh.indices = indices;
	return mesh;
}

Mesh join_meshes(Mesh m1, Mesh m2) {
	int index_offset = m1.vertex_data.size() / VERTEX_DATA_LEN;

	for (unsigned int i = 0; i < m2.vertex_data.size(); i++)
		m1.vertex_data.push_back(m2.vertex_data[i]);
	for (unsigned int i = 0; i < m2.indices.size(); i++)
		m1.indices.push_back(index_offset + m2.indices[i]);

	return m1;
}

void transform_mesh(Mesh* m, glm::mat4 transform) {
	// Transform coords
	for (unsigned int i = VERTEX_COORD_OFFSET; i < m->vertex_data.size(); i += VERTEX_DATA_LEN) {
		glm::vec3 coord(m->vertex_data[i], m->vertex_data[i + 1], m->vertex_data[i + 2]);
		coord = glm::vec3(transform * glm::vec4(coord, 1.0f));
		m->vertex_data[i] = coord.x;
		m->vertex_data[i + 1] = coord.y;
		m->vertex_data[i + 2] = coord.z;
	}

	glm::mat4 normal_transform = glm::transpose(glm::inverse(transform));
	for (unsigned int i = VERTEX_NORMAL_OFFSET; i < m->vertex_data.size(); i += VERTEX_DATA_LEN) {
		glm::vec3 coord(m->vertex_data[i], m->vertex_data[i + 1], m->vertex_data[i + 2]);
		coord = glm::normalize(glm::vec3(normal_transform * glm::vec4(coord, 1.0f)));
		m->vertex_data[i] = coord.x;
		m->vertex_data[i + 1] = coord.y;
		m->vertex_data[i + 2] = coord.z;
	}
}

Mesh* load_stl_mesh(std::string filepath, float r, float g, float b) {
	std::ifstream f;
	f.open(filepath.c_str());

	if (!f.is_open()) {
		error("load_stl_mesh()", "Failed to load mesh from " + filepath);
		return NULL;
	}

	const glm::vec3 color(r, g, b);
	Mesh* mesh = new Mesh;

	glm::vec3 normal, coords1, coords2, coords3;

	std::string tmp;
	while (f >> tmp) {
		if (tmp == "normal") {
			f >> normal.x >> normal.y >> normal.z;
		} else if (tmp == "vertex") {
			f >> coords1.x >> coords1.y >> coords1.z;
			f >> tmp >> coords2.x >> coords2.y >> coords2.z;
			f >> tmp >> coords3.x >> coords3.y >> coords3.z;
			add_triangle(mesh, coords1, normal, color, coords2, normal, color, coords3, normal, color);
		}
	}

	f.close();

	return mesh;
}

// ======== VERTEX OPERATIONS ========
int num_of_vertices(Mesh* m) {
	return m->vertex_data.size() / VERTEX_DATA_LEN;
}

glm::vec3 get_vertex_coords(Mesh* m, int vertex_num) {
	glm::vec3 coords;
	coords.x = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 0];
	coords.y = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 1];
	coords.z = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 2];
	return coords;
}

glm::vec3 get_vertex_normal(Mesh* m, int vertex_num) {
	glm::vec3 normal;
	normal.x = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 0];
	normal.y = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 1];
	normal.z = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 2];
	return normal;
}

glm::vec3 get_vertex_color(Mesh* m, int vertex_num) {
	glm::vec3 color;
	color.x = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 0];
	color.y = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 1];
	color.z = m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 2];
	return color;
}

void set_vertex_coords(Mesh* m, int vertex_num, glm::vec3 coords) {
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 0] = coords.x;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 1] = coords.y;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COORD_OFFSET + 2] = coords.z;
}

void set_vertex_normal(Mesh* m, int vertex_num, glm::vec3 normal) {
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 0] = normal.x;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 1] = normal.y;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_NORMAL_OFFSET + 2] = normal.z;
}

void set_vertex_color(Mesh* m, int vertex_num, glm::vec3 color) {
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 0] = color.x;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 1] = color.y;
	m->vertex_data[vertex_num * VERTEX_DATA_LEN + VERTEX_COLOR_OFFSET + 2] = color.z;
}

void add_vertex(Mesh* m, glm::vec3 coords, glm::vec3 normal, glm::vec3 color) {
	int new_vertex_num = num_of_vertices(m);
	m->vertex_data.resize(m->vertex_data.size() + VERTEX_DATA_LEN);
	set_vertex_coords(m, new_vertex_num, coords);
	set_vertex_normal(m, new_vertex_num, normal);
	set_vertex_color(m, new_vertex_num, color);
}

// ======== TRIANGLE OPERATIONS ========
int num_of_triangles(Mesh* m) {
	return m->indices.size() / 3;
}

glm::vec3 triangle_centroid(Mesh* m, int triangle_num) {
	glm::vec3 A = get_vertex_coords(m, m->indices[3 * triangle_num + 0]);
	glm::vec3 B = get_vertex_coords(m, m->indices[3 * triangle_num + 1]);
	glm::vec3 C = get_vertex_coords(m, m->indices[3 * triangle_num + 2]);
	return (A + B + C) / 3.0f;
}

float triangle_area(Mesh* m, int triangle_num) {
	glm::vec3 A = get_vertex_coords(m, m->indices[3 * triangle_num + 0]);
	glm::vec3 B = get_vertex_coords(m, m->indices[3 * triangle_num + 1]);
	glm::vec3 C = get_vertex_coords(m, m->indices[3 * triangle_num + 2]);

	glm::vec3 AB = B - A;
	glm::vec3 AC = C - A;

	return 0.5f * glm::length(glm::cross(AB, AC));
}

void add_triangle(Mesh* m, glm::vec3 coords_A, glm::vec3 normal_A, glm::vec3 color_A,
                           glm::vec3 coords_B, glm::vec3 normal_B, glm::vec3 color_B,
                           glm::vec3 coords_C, glm::vec3 normal_C, glm::vec3 color_C) {
	// Indices of new vertices
	int index_A = num_of_vertices(m);
	int index_B = index_A + 1;
	int index_C = index_A + 2;

	add_vertex(m, coords_A, normal_A, color_A);
	add_vertex(m, coords_B, normal_B, color_B);
	add_vertex(m, coords_C, normal_C, color_C);

	m->indices.push_back(index_A);
	m->indices.push_back(index_B);
	m->indices.push_back(index_C);
}

void subdivide_triangle(Mesh* m, int triangle_num) {
	int index_A = m->indices[3 * triangle_num + 0];
	int index_B = m->indices[3 * triangle_num + 1];
	int index_C = m->indices[3 * triangle_num + 2];
	int index_AB = num_of_vertices(m) + 0;
	int index_BC = num_of_vertices(m) + 1;
	int index_CA = num_of_vertices(m) + 2;

	glm::vec3 coords_A = get_vertex_coords(m, index_A);
	glm::vec3 normal_A = get_vertex_normal(m, index_A);
	glm::vec3 color_A = get_vertex_color(m, index_A);

	glm::vec3 coords_B = get_vertex_coords(m, index_B);
	glm::vec3 normal_B = get_vertex_normal(m, index_B);
	glm::vec3 color_B = get_vertex_color(m, index_B);

	glm::vec3 coords_C = get_vertex_coords(m, index_C);
	glm::vec3 normal_C = get_vertex_normal(m, index_C);
	glm::vec3 color_C = get_vertex_color(m, index_C);

	// Midpoints
	glm::vec3 coords_AB = (coords_A + coords_B) / 2.0f;
	glm::vec3 normal_AB = (normal_A + normal_B) / 2.0f;
	glm::vec3 color_AB = (color_A + color_B) / 2.0f;

	glm::vec3 coords_BC = (coords_B + coords_C) / 2.0f;
	glm::vec3 normal_BC = (normal_B + normal_C) / 2.0f;
	glm::vec3 color_BC = (color_B + color_C) / 2.0f;

	glm::vec3 coords_CA = (coords_C + coords_A) / 2.0f;
	glm::vec3 normal_CA = (normal_C + normal_A) / 2.0f;
	glm::vec3 color_CA = (color_C + color_A) / 2.0f;

	// Add vertices
	add_vertex(m, coords_AB, normal_AB, color_AB);
	add_vertex(m, coords_BC, normal_BC, color_BC);
	add_vertex(m, coords_CA, normal_CA, color_CA);

	// Add triangles
	m->indices[3 * triangle_num + 0] = index_A;
	m->indices[3 * triangle_num + 1] = index_AB;
	m->indices[3 * triangle_num + 2] = index_CA;

	m->indices.push_back(index_AB);
	m->indices.push_back(index_B);
	m->indices.push_back(index_BC);

	m->indices.push_back(index_BC);
	m->indices.push_back(index_CA);
	m->indices.push_back(index_AB);

	m->indices.push_back(index_CA);
	m->indices.push_back(index_BC);
	m->indices.push_back(index_C);
}

void subdivide_all_triangles(Mesh* m) {
	int n = num_of_triangles(m);

	for (int i = 0; i < n; i++) {
		subdivide_triangle(m, i);
	}
}

// ================ 2D MESH GENERATORS ================
Mesh make_square_mesh(unsigned int n, float r, float g, float b) {
	Mesh square;
	const float sidelength = 2.0f;
	const float step = sidelength / n;
	const glm::vec3 color(r, g, b);
	const glm::vec3 normal(0.0f, 1.0f, 0.0f);

	// Vertices
	for (unsigned int i = 0; i < n + 1; i++) {
		for (unsigned int j = 0; j < n + 1; j++) {
			glm::vec3 coords(-0.5f * sidelength + j * step, 0.0f, 0.5f * sidelength - i * step);
			add_vertex(&square, coords, normal, color);
		}
	}

	// Indices
	for (unsigned int i = 0; i < n; i++) {
		for (unsigned int j = 0; j < n; j++) {
			square.indices.push_back((n+1) * i + j);
			square.indices.push_back((n+1) * (i+1) + j);
			square.indices.push_back((n+1) * i + (j+1));

			square.indices.push_back((n+1) * i + (j+1));
			square.indices.push_back((n+1) * (i+1) + j);
			square.indices.push_back((n+1) * (i+1) + (j+1));
		}
	}

	return square;
}

Mesh make_circle_mesh(unsigned int n, float r, float g, float b) {
	Mesh circle;
	const float radius = 1.0f;
	const double delta_phi = 2 * M_PI / n;
	double phi = 0.0;

	// Center vertex
	circle.vertex_data = {
		0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   r, g, b
	};

	// Outer vertices
	for (unsigned int i = 0; i < n; i++) {
		float x = radius * cos(phi);
		float y = 0.0f;
		float z = radius * sin(phi);

		circle.vertex_data.push_back(x);
		circle.vertex_data.push_back(y);
		circle.vertex_data.push_back(z);

		circle.vertex_data.push_back(0.0f);
		circle.vertex_data.push_back(1.0f);
		circle.vertex_data.push_back(0.0f);

		circle.vertex_data.push_back(r);
		circle.vertex_data.push_back(g);
		circle.vertex_data.push_back(b);
		
		phi += delta_phi;
	}

	// Indices
	for (unsigned int i = 0; i < n - 1; i++) {
		circle.indices.push_back(i + 2);
		circle.indices.push_back(i + 1);
		circle.indices.push_back(0);
	}

	circle.indices.push_back(n);
	circle.indices.push_back(1);
	circle.indices.push_back(0);

	return circle;
}

// ================ 3D MESH GENERATORS ================
Mesh make_truncated_cone_mesh(unsigned int n, float top_radius, float r, float g, float b) {
	Mesh trcone;
	const float base_radius = 1.0f;
	const double delta_phi = 2 * M_PI / n;
	double phi = 0.0;

	// Vertices
	for (unsigned int i = 0; i < n; i++) {
		float bot_x = base_radius * cos(phi);
		float bot_y = -1.0f;
		float bot_z = base_radius * sin(phi);

		trcone.vertex_data.push_back(bot_x);
		trcone.vertex_data.push_back(bot_y);
		trcone.vertex_data.push_back(bot_z);
		
		glm::vec3 normal(cos(phi), base_radius - top_radius, sin(phi));
		normal = glm::normalize(normal);
		trcone.vertex_data.push_back(normal.x);
		trcone.vertex_data.push_back(normal.y);
		trcone.vertex_data.push_back(normal.z);

		trcone.vertex_data.push_back(r);
		trcone.vertex_data.push_back(g);
		trcone.vertex_data.push_back(b);
		
		phi += delta_phi;
	}

	for (unsigned int i = 0; i < n; i++) {
		float top_x = top_radius * cos(phi);
		float top_y = 1.0f;
		float top_z = top_radius * sin(phi);

		trcone.vertex_data.push_back(top_x);
		trcone.vertex_data.push_back(top_y);
		trcone.vertex_data.push_back(top_z);
		
		glm::vec3 normal(cos(phi), base_radius - top_radius, sin(phi));
		normal = glm::normalize(normal);
		trcone.vertex_data.push_back(normal.x);
		trcone.vertex_data.push_back(normal.y);
		trcone.vertex_data.push_back(normal.z);

		trcone.vertex_data.push_back(r);
		trcone.vertex_data.push_back(g);
		trcone.vertex_data.push_back(b);
		
		phi += delta_phi;
	}

	// Indices
	for (unsigned int i = 0; i < n - 1; i++) {
		trcone.indices.push_back(i);
		trcone.indices.push_back(n + i);
		trcone.indices.push_back(n + i + 1);

		trcone.indices.push_back(i);
		trcone.indices.push_back(n + i + 1);
		trcone.indices.push_back(i + 1);
	}

	trcone.indices.push_back(n - 1);
	trcone.indices.push_back(2 * n - 1);
	trcone.indices.push_back(n);

	trcone.indices.push_back(n - 1);
	trcone.indices.push_back(n);
	trcone.indices.push_back(0);

	// Add bases
	Mesh top_base = make_circle_mesh(n, r, g, b);
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::scale(transform, glm::vec3(top_radius, 1.0f, top_radius));
	transform_mesh(&top_base, transform);

	Mesh bot_base = make_circle_mesh(n, r, g, b);
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	transform = glm::rotate(transform, (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
	transform_mesh(&bot_base, transform);

	trcone = join_meshes(trcone, top_base);
	trcone = join_meshes(trcone, bot_base);

	return trcone;
}

Mesh make_uv_sphere_mesh(unsigned int slices, unsigned int stacks, float r, float g, float b) {
	Mesh sphere;
	const float radius = 1.0f;
	const double delta_phi = 2 * M_PI / slices;
	const double delta_theta = M_PI / stacks;
	double phi = 0.0f;
	double theta = delta_theta;

	sphere.vertex_data = {
		0.0f,  radius, 0.0f,   0.0f,  1.0f, 0.0f,   r, g, b, // Top pole
		0.0f, -radius, 0.0f,   0.0f, -1.0f, 0.0f,   r, g, b, // Bottom pole
	};

	for (unsigned int i = 0; i < stacks - 1; i++) {
		float x, y, z;

		for (unsigned int j = 0; j < slices; j++) {
			// Polar to rectangular
			y = radius * cos(theta);
			x = radius * sin(theta) * cos(phi);
			z = radius * sin(theta) * sin(phi);

			// Coords
			sphere.vertex_data.push_back(x);
			sphere.vertex_data.push_back(y);
			sphere.vertex_data.push_back(z);

			// Normal
			sphere.vertex_data.push_back(x);
			sphere.vertex_data.push_back(y);
			sphere.vertex_data.push_back(z);

			// Color
			sphere.vertex_data.push_back(r);
			sphere.vertex_data.push_back(g);
			sphere.vertex_data.push_back(b);

			phi += delta_phi;
		}

		theta += delta_theta;
	}

	// Top and bottom poles
	for (unsigned int i = 2; i < slices + 1; i++) {
		sphere.indices.push_back(i);
		sphere.indices.push_back(0);
		sphere.indices.push_back(i + 1);

		sphere.indices.push_back((stacks - 2) * slices + i);
		sphere.indices.push_back(1);
		sphere.indices.push_back((stacks - 2) * slices + i + 1);
	}

	sphere.indices.push_back(slices + 1);
	sphere.indices.push_back(0);
	sphere.indices.push_back(2);

	sphere.indices.push_back((stacks - 2) * slices + slices + 1);
	sphere.indices.push_back(1);
	sphere.indices.push_back((stacks - 2) * slices + 2);

	// Sides
	for (unsigned int i = 1; i < stacks - 1; i++) {
		for (unsigned int j = 0; j < slices - 1; j++) {
			sphere.indices.push_back(i * slices + j + 2);
			sphere.indices.push_back((i - 1) * slices + j + 2);
			sphere.indices.push_back(i * slices + (j + 1) + 2);

			sphere.indices.push_back(i * slices + (j + 1) + 2);
			sphere.indices.push_back((i - 1) * slices + j + 2);
			sphere.indices.push_back((i - 1) * slices + (j + 1) + 2);
		}

		sphere.indices.push_back(i * slices + 2);
		sphere.indices.push_back(i * slices + 2 + (slices - 1));
		sphere.indices.push_back(i * slices + 2 - 1);

		sphere.indices.push_back(i * slices + 2);
		sphere.indices.push_back(i * slices + 2 - 1);
		sphere.indices.push_back((i - 1) * slices + 2);
	}

	return sphere;
}

Mesh make_ico_sphere_mesh(unsigned int num_of_subdivisions, float r, float g, float b) {
	// Start from low-poly uv sphere
	Mesh sphere = make_uv_sphere_mesh(6, 3, r, g, b);

	for (unsigned int i = 0; i < num_of_subdivisions; i++) {
		//debug("make_ico_sphere_mesh()", "Level: " + std::to_string(i) + " (" + std::to_string(num_of_triangles(&sphere)) + " triangles)");
		subdivide_all_triangles(&sphere);
	}

	// Project all vertices onto unit sphere
	for (int i = 0; i < num_of_vertices(&sphere); i++) {
		set_vertex_coords(&sphere, i, glm::normalize(get_vertex_coords(&sphere, i)));
	}

	return sphere;
}

