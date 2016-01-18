#ifndef __MESH_H
#define __MESH_H

#include <glm/glm.hpp>

class Mesh
{
public:
	Mesh(const glm::vec3 &wp = glm::vec3(0.0));
	virtual void load() = 0;
	virtual void draw() const = 0;
	glm::mat4 getModelToWorldMat() const;

	glm::vec3 getWorldPos() const;
	void setWorldPos(const glm::vec3 &wp);
	virtual ~Mesh() {}
protected:
	glm::vec3 worldPos;
};

#endif