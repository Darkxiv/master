#version 330

uniform samplerCube skybox;

in vec3 texCo;

out vec4 outputColor;

void main()
{
	outputColor = texture(skybox, -texCo);
}
