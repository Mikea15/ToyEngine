#version 420 core
out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	// FragColor = vec4(lightColor, 1.0);
}
