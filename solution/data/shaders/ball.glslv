#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec2 texCoord;
out vec3 cameraNormal;
out vec3 worldNormal;
out vec3 cameraSpacePos;
out vec3 worldSpacePos;

out vec3 lightingNormal;
out vec3 lightingEyeVec;
out vec3 lightingPos;

layout(std140) uniform GlobalMatrices
{
	mat4 cameraToClipMatrix;
	mat4 worldToCameraMatrix;
};

uniform mat4 modelToWorldMatrix;
uniform mat3 normalModelToCameraMatrix;
uniform mat3 normalModelToWorldMatrix;

uniform vec3 camPos;
uniform mat4 worldToLightMatrix;
uniform mat3 worldToLightITMatrix;


void main()
{
	vec4 tempPos = modelToWorldMatrix * vec4(position, 1.0);
	worldSpacePos = vec3(tempPos);
	tempPos = worldToCameraMatrix * tempPos;
	gl_Position = cameraToClipMatrix * tempPos;

	texCoord = texcoord;
	cameraNormal = normalize(normalModelToCameraMatrix * normal);
	worldNormal = normalize(normalModelToWorldMatrix * normal);
	cameraSpacePos = vec3(tempPos);

	vec4 lightingEyePos = worldToLightMatrix * vec4(camPos, 1.0);
	vec4 lp = worldToLightMatrix * vec4(worldSpacePos, 1.0);
	lightingEyeVec = vec3(lightingEyePos - lp);
	lightingNormal = vec3(worldToLightITMatrix * worldNormal);
	lightingPos = vec3(worldToLightMatrix * vec4(worldSpacePos, 1.0));
}
