#ifndef __BALL_H
#define __BALL_H

#include "settings.h"
#include "mesh.h"
#include <GL/glew.h>
#include <glm/gtx/quaternion.hpp>

class Sphere: public Mesh
{
public:
	Sphere(const glm::vec3 &wp = glm::vec3(0.0), int r = SPHERE_SHAPE, int s = SPHERE_SHAPE);

	virtual void load();
	virtual void draw() const;

	glm::mat4 getModelToWorldMat() const;
	glm::vec3 getVelocity() const;

	void changeVelocity(const glm::vec3 &a, float frameDiv);
	void move(float frameDiv);
	void setVelocity(const glm::vec3 &v);
	virtual ~Sphere();
private:
	const int rings;
	const int sectors;

	GLuint vao;
	GLsizei vaoSize;
	GLuint vertexBufferObject;
	GLuint indexBufferObject;

	glm::vec3 velocity;
	glm::quat rotation;

	const float maxSpeed;
	const float braking;
	const float acceleration;
};

class Plane: public Mesh
{
public:
	Plane(const glm::vec3 &wp = glm::vec3(0.0));

	virtual void load();
	virtual void draw() const;

	glm::mat4 getModelToWorldMat() const;
	glm::vec2 getTextureScale() const;
	void setScale(const glm::vec3 &sc);
	void setTextureScale(const glm::vec2 &tsc);
	void setRotate(const glm::vec3 &euler);
	virtual ~Plane();
private:
	GLuint vao;
	GLsizei vaoSize;
	GLuint vertexBufferObject;
	GLuint indexBufferObject;	

	glm::vec3 scale;
	glm::vec2 textureScale;
	glm::quat rotation;


};

class Cube: public Mesh
{
public:
	Cube(const glm::vec3 &wp = glm::vec3(0.0));

	virtual void load();
	virtual void draw() const;

	glm::mat4 getModelToWorldMat() const;
	void setScale(const glm::vec3 &sc);
	virtual ~Cube();
private:
	GLuint vao;
	GLsizei vaoSize;
	GLuint vertexBufferObject;
	GLuint indexBufferObject;	

	glm::vec3 scale;
};

#endif