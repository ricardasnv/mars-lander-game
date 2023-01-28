// Ricardas Navickas 2020
// Shader for world objects
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 object_color;
out vec3 object_normal;
out float normal_length;
out vec3 FragPos;

uniform vec3 view_pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	FragPos = view_pos + vec3(model * vec4(aPos, 1.0f));
	object_normal = mat3(transpose(inverse(model))) * aNormal;
	object_color = aColor;
	normal_length = length(aNormal);
}

