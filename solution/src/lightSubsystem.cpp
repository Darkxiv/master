#include "lightSubsystem.h"

LightSubsystem::LightSubsystem()
{
	const glm::vec4 lightPos[] = { 
		glm::vec4(3.0, 8.0, 3.0, 1.0),
		glm::vec4(-3.0, 8.0, 3.0, 1.0),
		glm::vec4(0.0, 7.0, -4.0, 1.0),
	};

	const glm::vec4 lightIntensity[] = { 
		glm::vec4(0.85, 0.9, 1.0, 1.0),
		glm::vec4(0.85, 0.9, 1.0, 1.0),
		glm::vec4(1.0, 0.85, 0.45, 1.0),
	};

	const float halfLightDistance = 7.0f;
	const float lightAttenuation = 1.0f / (halfLightDistance * halfLightDistance);

	lightData.ambientIntensity = glm::vec4(glm::vec3(0.3f), 1.0f);
	lightData.lightAttenuation = lightAttenuation;

	for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		lightsWorldPos[i] = lightPos[i];
		lightData.lights[i].lightIntensity = lightIntensity[i];
	}
}

LightBlock LightSubsystem::getLightInformation(const glm::mat4 &worldToCameraMat)
{
	for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
		lightData.lights[i].cameraSpaceLightPos = worldToCameraMat * lightsWorldPos[i];
	return lightData;
}

std::vector<glm::vec3> LightSubsystem::getLightWorldPosition() const
{
	std::vector<glm::vec3> retV;
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
		retV.push_back(glm::vec3(lightsWorldPos[i]));
	return retV;
}

void LightSubsystem::setLightIntesity(int index, const glm::vec4 &intesity)
{
	if (index < 0 || index > NUMBER_OF_LIGHTS)
		return;
	lightData.lights[index].lightIntensity = glm::clamp(intesity, 0.0f, 1.0f);
}

void LightSubsystem::setLightWorldPos(int index, const glm::vec4 &intesity)
{
	if (index < 0 || index > NUMBER_OF_LIGHTS)
		return;
	lightsWorldPos[index] = intesity;
}
