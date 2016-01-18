#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

const int numberOfLights = 3;

out vec2 texCoord;
out vec3 vertexNormal;
out vec3 cameraSpacePosition;
out vec4 lightPos[numberOfLights];

layout(std140) uniform GlobalMatrices
{
	mat4 cameraToClipMatrix;
	mat4 worldToCameraMatrix;
};

uniform vec2 textureScale;
uniform mat4 modelToWorldMatrix;
uniform mat3 normalModelToCameraMatrix;
uniform mat4 modelToLightToClipMatrix[numberOfLights];

void main()
{
	vec4 worldPosition =  modelToWorldMatrix * vec4(position, 1.0);
	vec4 tempPosition =  worldToCameraMatrix * worldPosition;
	gl_Position = cameraToClipMatrix * tempPosition;

	texCoord = texcoord * textureScale;
	vertexNormal = normalize(normalModelToCameraMatrix * normal);
	cameraSpacePosition = vec3(tempPosition);
	
	for (int i = 0; i < numberOfLights; i++)
		lightPos[i] = modelToLightToClipMatrix[i] * worldPosition;
}
