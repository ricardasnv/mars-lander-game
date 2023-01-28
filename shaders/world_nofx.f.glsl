// Ricardas Navickas 2020
// Shader for world objects with no lighting (light sources, UI elements etc.)
#version 330 core
out vec4 FragColor;

in vec3 object_color;

void main(void) {
	FragColor = vec4(object_color, 1.0f);
}
