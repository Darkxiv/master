#ifndef __MATERIAL_H
#define __MATERIAL_H

#include <glm/glm.hpp>

struct MaterialBlock
{
	glm::vec4 specularColor;
	float specularShininess;
	float reflectivity;
	//float padding[2];
};


#endif