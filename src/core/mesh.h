// Ricardas Navickas 2020
#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "glm/glm.hpp"

#define VERTEX_DATA_LEN 9 // number of floats per vertex
#define VERTEX_COORD_OFFSET 0
#define VERTEX_NORMAL_OFFSET 3
#define VERTEX_COLOR_OFFSET 6

struct Mesh {
	// Vertices, normals & colors (VNCVNCVNCVNC)
	// Each vertex/normal/color is represented by a 3-vector
	std::vector<float> vertex_data;
	
	// Triangles
	std::vector<unsigned int> indices;
};

Mesh make_mesh(std::vector<float> vertex_data, std::vector<unsigned int> indices);
Mesh join_meshes(Mesh m1, Mesh m2);
void transform_mesh(Mesh* m, glm::mat4 transform);

// Load mesh on heap from file
Mesh* load_stl_mesh(std::string filepath, float r, float g, float b);

// Operations with vertices
int num_of_vertices(Mesh* m);
glm::vec3 get_vertex_coords(Mesh* m, int vertex_num);
glm::vec3 get_vertex_normal(Mesh* m, int vertex_num);
glm::vec3 get_vertex_color(Mesh* m, int vertex_num);
void set_vertex_coords(Mesh* m, int vertex_num, glm::vec3 coords);
void set_vertex_normal(Mesh* m, int vertex_num, glm::vec3 normal);
void set_vertex_color(Mesh* m, int vertex_num, glm::vec3 color);
void add_vertex(Mesh* m, glm::vec3 coords, glm::vec3 normal, glm::vec3 color);

// Operations with triangles
int num_of_triangles(Mesh* m);
glm::vec3 triangle_centroid(Mesh* m, int triangle_num);
float triangle_area(Mesh* m, int triangle_num);
void add_triangle(Mesh* m, glm::vec3 coords_A, glm::vec3 normal_A, glm::vec3 color_A,
                           glm::vec3 coords_B, glm::vec3 normal_B, glm::vec3 color_B,
                           glm::vec3 coords_C, glm::vec3 normal_C, glm::vec3 color_C);
void subdivide_triangle(Mesh* m, int triangle_num);
void subdivide_all_triangles(Mesh* m); // Subdivides each triangle in mesh into 4 equal smaller ones

// 2D mesh generators. X,Z coords range from -1.0 to 1.0 and y=0.0. Normal vectors point in the +Y direction
Mesh make_square_mesh(unsigned int n, float r, float g, float b); // each side has n+1 vertices
Mesh make_circle_mesh(unsigned int n, float r, float g, float b); // approximated by n-sided polygon

// 3D mesh generators. All generated shapes are scaled so that X,Y,Z coords range from -1.0 to 1.0
Mesh make_truncated_cone_mesh(unsigned int n, float top_radius, float r, float g, float b); // base radius is 1.0
Mesh make_uv_sphere_mesh(unsigned int slices, unsigned int stacks, float r, float g, float b);
Mesh make_ico_sphere_mesh(unsigned int num_of_subdivisions, float r, float g, float b);

#endif
