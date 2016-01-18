#version 330

layout(location = 0) in vec3 position;

layout(std140) uniform GlobalMatrices
{
	mat4 cameraToClipMatrix;
	mat4 worldToCameraMatrix;
};

uniform mat4 modelToWorldMatrix;

out vec3 texCo;

void main()
{
	mat4 worldToCameraMatrix434 = mat4(mat3(worldToCameraMatrix));
	vec4 pos = cameraToClipMatrix * worldToCameraMatrix434 * vec4(position, 1.0);
	texCo = vec3(modelToWorldMatrix * vec4(position, 1.0));
    gl_Position = pos.xyww;
}
