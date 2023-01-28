// Ricardas Navickas 2020
// Shader for world objects
#version 330 core
#define MAX_NUM_OF_LIGHTS 8
in vec3 FragPos;
out vec4 FragColor;

in vec3 object_color;
in vec3 object_normal;
in float normal_length;

uniform bool apply_fog = true;
uniform float fog_density = 0.0f;
uniform vec3 fog_color = vec3(0.7f, 0.5f, 0.5f);

uniform vec3 view_pos;

uniform bool apply_lighting = true;
uniform int num_of_lights = 0;
uniform vec3 light_pos[MAX_NUM_OF_LIGHTS];
uniform vec3 light_color[MAX_NUM_OF_LIGHTS];
uniform float specular_coefficient = 0.0f;
uniform int specular_exponent = 0;

const vec3 ambient_color = vec3(1.0f, 1.0f, 1.0f);

vec3 light();
float distance_from_camera();
float fog_factor();

void main(void) {
	float alpha = 1.0f;

	vec3 result = object_color;
	if (apply_lighting == true) {
		result = result * light();
	} else {
		// If lighting is disabled, use the normal vector to determine alpha
		alpha = normal_length;
	}

	if (apply_fog == true)
		result = mix(result, fog_color, fog_factor());

	FragColor = vec4(result, alpha);
}

vec3 light() {
	vec3 total_light = vec3(0.0f, 0.0f, 0.0f);

	// **** Ambient lighting ****
	float ambient_strength = 0.3f;
	vec3 ambient = ambient_strength * ambient_color;
	total_light += ambient;

	// **** Diffuse & specular lighting ****
	for (int i = 0; i < num_of_lights; i++) {
		vec3 norm = normalize(object_normal);
		vec3 lightDir = normalize(light_pos[i] - FragPos);
		vec3 viewDir = normalize(view_pos - FragPos);

		// Diffuse lighting
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = diff * light_color[i];

		// Specular lighting
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0f), specular_exponent);
		vec3 specular = specular_coefficient * spec * light_color[i];

		total_light += diffuse + specular;
	}

	return total_light;
}

float distance_from_camera() {
	return length(FragPos - view_pos);
}

float fog_factor() {
	return clamp((fog_density / 0.008e9) * (distance_from_camera() / 10), 0.0f, 1.0f);
	//return clamp(1 - exp(-pow(fog_density * 0.006 * distance_from_camera(), 2)), 0.0f, 1.0f);
}

