#include "sceneObjects.h"
#include "settings.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>


Sphere::Sphere(const glm::vec3 &wp, int r, int s): Mesh(wp), 
	maxSpeed(0.3f), braking(0.001f), 
	acceleration(0.005f), rings(r), sectors(s) 
{ }

void Sphere::load()
{
	float const R = 1.0f / (float)(rings-1);
	float const S = 1.0f / (float)(sectors-1);
	size_t sizeOfVertexData = rings * sectors * 8;
	size_t sizeOfIndexData = rings * sectors * 4;
	float *sphereVertexData = new GLfloat[sizeOfVertexData];
	GLshort *sphereIndexData = new GLshort[sizeOfIndexData];
	size_t normalDataOffset = sizeof(float) * rings * sectors * 3;
	size_t texcoDataOffset = sizeof(float) * rings * sectors * 3 * 2;

	float *v = sphereVertexData;
	float *n = v + normalDataOffset / sizeof(float);
	float *t = v + texcoDataOffset / sizeof(float);
	GLshort *i = sphereIndexData;

	for (int r = 0; r < rings; r++)
		for (int s = 0; s < sectors; s++) 
		{
			float y = sin(-M_PI / 2.0f + M_PI * r * R);
			float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			*v++ = x;
			*v++ = y;
			*v++ = z;

			*n++ = x;
			*n++ = y;
			*n++ = z;

			*t++ = -s*S;
			*t++ = r*R;

			*i++ = (r+1) * sectors + s;
			*i++ = (r+1) * sectors + (s+1);
			*i++ = r * sectors + (s+1);
			*i++ = r * sectors + s;
		}

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sizeOfVertexData, sphereVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLshort) * sizeOfIndexData, sphereIndexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	for (int i = 0; i <= 2; i++)
		glEnableVertexAttribArray(i);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalDataOffset);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)texcoDataOffset);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBindVertexArray(0);

	vaoSize = sizeOfIndexData;

	delete[] sphereVertexData;
	delete[] sphereIndexData;
}

void Sphere::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_QUADS, vaoSize, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

glm::mat4 Sphere::getModelToWorldMat() const
{
	return glm::translate(glm::mat4(1.0), worldPos) * glm::mat4_cast(rotation);
}

glm::vec3 Sphere::getVelocity() const
{
	return velocity;
}

void Sphere::changeVelocity(const glm::vec3 &a, float frameDiv)
{
	velocity += glm::normalize(glm::vec3(a.x, 0.0, a.z)) * acceleration * frameDiv;
	velocity = glm::clamp(velocity, -maxSpeed, maxSpeed);
}

void Sphere::move(float frameDiv)
{
	float angle = glm::length(velocity) * frameDiv;
	if (angle > EPS)
	{
		glm::quat rq = glm::normalize(glm::angleAxis(angle, glm::normalize(glm::vec3(velocity.z, 0.0, -velocity.x))));
		rotation = glm::normalize(rq * rotation);
	}

	worldPos += velocity * frameDiv;
	
	const float a = braking * frameDiv;
	velocity.x = velocity.x < 0.0 ? glm::clamp(velocity.x + a, -maxSpeed, 0.0f) : glm::clamp(velocity.x - a, 0.0f, maxSpeed);
	velocity.z = velocity.z < 0.0 ? glm::clamp(velocity.z + a, -maxSpeed, 0.0f) : glm::clamp(velocity.z - a, 0.0f, maxSpeed);
}

void Sphere::setVelocity(const glm::vec3 &v)
{
	velocity = v;
}

Sphere::~Sphere()
{
	glDeleteBuffers(1, &vertexBufferObject);
	glDeleteBuffers(1, &indexBufferObject);
}

Plane::Plane(const glm::vec3 &wp): Mesh(wp), scale(glm::vec3(1.0)), textureScale(glm::vec2(1.0)) { }

void Plane::load()
{
	const float planeVertexData[] = {
		-1.0f, 0.0f, -1.0f,
		1.0f, 0.0f, -1.0f,
		-1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,

		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,

		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};

	const GLshort planeIndexData[] = 
	{
		2, 1, 0,
		2, 3, 1,
	};

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertexData), planeVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndexData), planeIndexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	size_t normalDataOffset = sizeof(float) * 3 * 4;
	size_t texcoDataOffset = sizeof(float) * 3 * 8;
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	for (int i = 0; i <= 2; i++)
		glEnableVertexAttribArray(i);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)normalDataOffset);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)texcoDataOffset);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBindVertexArray(0);

	vaoSize = sizeof(planeIndexData) / sizeof(planeIndexData[0]);
}

void Plane::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, vaoSize, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

glm::mat4 Plane::getModelToWorldMat() const
{
	return glm::scale(glm::translate(glm::mat4(1.0), worldPos) * glm::mat4_cast(rotation), scale);
}

glm::vec2 Plane::getTextureScale() const
{
	return textureScale;
}

void Plane::setScale(const glm::vec3 &sc)
{
	scale = sc;
}

void Plane::setTextureScale(const glm::vec2 &tsc)
{
	textureScale = tsc;
}

void Plane::setRotate(const glm::vec3 &euler)
{
	rotation = glm::quat(euler);
}

Plane::~Plane()
{
	glDeleteBuffers(1, &vertexBufferObject);
	glDeleteBuffers(1, &indexBufferObject);
}

Cube::Cube(const glm::vec3 &wp): Mesh(wp), scale(glm::vec3(1.0)) {}

void Cube::load()
{
	const float cubeVertexData[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
	};

	const GLshort cubeIndexData[] = 
	{
		0, 2, 1,
		0, 3, 2,

		1, 2, 6,
		1, 6, 5,

		5, 6, 4,
		6, 7, 4,

		7, 3, 4,
		3, 0, 4,

		3, 6, 2,
		3, 7, 6,

		0, 1, 5,
		0, 5, 4,
	};

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexData), cubeVertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndexData), cubeIndexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBindVertexArray(0);

	vaoSize = sizeof(cubeIndexData) / sizeof(cubeIndexData[0]);
}

void Cube::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, vaoSize, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

glm::mat4 Cube::getModelToWorldMat() const
{
	return glm::scale(glm::translate(glm::mat4(1.0), worldPos), scale);
}

void Cube::setScale(const glm::vec3 &sc)
{
	scale = sc;
}

Cube::~Cube()
{
	glDeleteBuffers(1, &vertexBufferObject);
	glDeleteBuffers(1, &indexBufferObject);
}