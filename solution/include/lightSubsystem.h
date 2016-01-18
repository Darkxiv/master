#ifndef __LIGHT_H
#define __LIGHT_H

#include <vector>
#include <glm/glm.hpp>

struct PerLight
{
	glm::vec4 cameraSpaceLightPos;
	glm::vec4 lightIntensity;
};

const int NUMBER_OF_LIGHTS = 3;

struct LightBlock
{
	glm::vec4 ambientIntensity;
	float lightAttenuation;
	float pad[3];
	PerLight lights[NUMBER_OF_LIGHTS];
};

class LightSubsystem
{
public:
	LightSubsystem();
	
	LightBlock getLightInformation(const glm::mat4 &worldToCameraMat);
	std::vector<glm::vec3> getLightWorldPosition() const;

	void setLightIntesity(int index, const glm::vec4 &intesity);
	void setLightWorldPos(int index, const glm::vec4 &worldPos);
private:
	LightBlock lightData;
	glm::vec4 lightsWorldPos[NUMBER_OF_LIGHTS];
};

#endif