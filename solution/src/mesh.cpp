#include "mesh.h"
#include <glm/gtc/matrix_transform.hpp>

Mesh::Mesh(const glm::vec3 &wp): worldPos(wp) {}

glm::mat4 Mesh::getModelToWorldMat() const
{
	return glm::translate(glm::mat4(1.0), worldPos);
}

glm::vec3 Mesh::getWorldPos() const
{
	return worldPos;
}

void Mesh::setWorldPos(const glm::vec3 &wp)
{
	worldPos = wp;
}