#ifndef __GRAPHICS_SYBSYSTEM_H
#define __GRAPHICS_SYBSYSTEM_H

#include "lightSubsystem.h"
#include "material.h"
#include "sceneObjects.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>

class GraphicsSubsystem
{
public:
	GraphicsSubsystem();
	int initGraphicsSubsystem();
	void reshape(int w, int h);

	glm::vec3 getViewVector();
	void setCamTarget(const glm::vec3 &camPos);
	void rotateCam(const glm::vec3 &diff);

	void shadowMapPass(const Mesh *target, LightSubsystem &lss);
	void drawBall(const Sphere &ball);
	void drawPlane(const Plane &plane, const std::string &textureName = "cloth");
	void drawLight(const Mesh *reference, LightSubsystem &lss);
	void drawSkybox(const Cube &cube);

	std::string getClothTexture();
	std::string getWoodTexture();
	void bindLighting(LightSubsystem &lss);
	void bindMaterial(const MaterialBlock &matData);
	void setCam();
	void swapBuffers();
	void accumFrame(int cur, int n);
	void returnFrame();
	void clearBuffers();
	~GraphicsSubsystem();
private:
	const float IBLscale;
	const float zNear;
	const float zFar;
	const float minCamAngle;
	const float maxCamAngle;
	const float minCamDistance;
	const float maxCamDistance;

	LightSubsystem lss;

	glm::ivec2 windowSize;
	glm::vec3 sphereCamRelPos;
	glm::vec3 camTarget;
	glm::vec3 camPos;
	glm::vec3 viewVector;
	glm::mat4 worldToCam;

	std::map<std::string, GLuint> shaders;
	std::map<std::string, GLuint> bindingIndexes;
	std::map<std::string, GLuint> texUnits;
	std::map<std::string, GLuint> uniformBuffers;
	std::map<std::string, GLuint> textures;

	GLuint sampler;
	GLuint shadowMapTextures[NUMBER_OF_LIGHTS];
	GLuint shadowFbo[NUMBER_OF_LIGHTS];
	GLint shadowTexUnit[NUMBER_OF_LIGHTS];
	glm::mat4 modelLightWorldClip[NUMBER_OF_LIGHTS];
	std::map<GLenum, std::map<std::string, GLuint> > programUniforms;

	void createDepthBuffer();
	void reallocShadowTextures();
	void createSampler();
	void loadShaders();
	void loadUniforms();
	void loadBuffers();
	void loadTexture(const char *filename, GLuint &texture);
	void loadCubemap(const char *filenames[], int csize, GLuint &texture);

	glm::vec3 resolveCamPosition();
	glm::mat4 calcLookAtMatrix(const glm::vec3 &cameraPt, const glm::vec3 &lookPt, const glm::vec3 &upPt);
	void loadUniforms(GLuint pr, const char *uniforms[], int usize, const char *blocks[], int bsize);
	void loadTextureUnits(const char *textureUnits[], int tsize);
};

#endif