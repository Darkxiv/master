#include "graphicsSubsystem.h"
#include "settings.h"
#include "shaderWorker.h"
#include "lightSubsystem.h"
#include "material.h"

#include <algorithm>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glimg/glimg.h>

#define loadSky 1

GraphicsSubsystem::GraphicsSubsystem(): sphereCamRelPos(295.0f, -73.0f, 4.0f), 
	camTarget(0.0f, 1.0f, 0.0f), 
	windowSize(WIN_W, WIN_H), 
	zNear(1.0f),	zFar(100.0f), IBLscale(0.07f),
	minCamAngle(-87.0f), maxCamAngle(-1.0f),
	minCamDistance(3.0f), maxCamDistance(12.0f)
{ }

int GraphicsSubsystem::initGraphicsSubsystem()
{
	char *myargv[1];
	int myargc = 1;
	myargv[0] = _strdup(COPYRIGHT);
	glutInit(&myargc, myargv);
	glutInitWindowPosition(WIN_POS_X, WIN_POS_Y);
	glutInitWindowSize(windowSize.x, windowSize.y);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutCreateWindow("Practical Work");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	
	{
		printf("Errors occurred during glew init\n");
		return GSS_ERROR;
	}
	else if (!GLEW_VERSION_3_3)
	{
		GLint major = 0, minor = 0;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		printf("Application requires OpenGL 3.3; current version is %i.%i\n", major, minor);
		return GSS_ERROR;
	}

	bindingIndexes["matrices"] = 0;
	bindingIndexes["light"] = 1;
	bindingIndexes["material"] = 2;

	const char *textureUnits[] = { "ball", "cloth", "wood", "room", "roomBall" };
	loadTextureUnits(textureUnits, sizeof(textureUnits) / sizeof(char*));

	loadShaders();
	loadUniforms();
	loadBuffers();
	
	printf("Loading textures...\n");
	loadTexture(TEXTURE_PATH "ball_albedo.png", textures["ball"]);
	loadTexture(TEXTURE_PATH "cloth.png", textures["cloth"]);
	loadTexture(TEXTURE_PATH "wood.png", textures["wood"]);

#if loadSky == 1
	const char *skybox[] = { TEXTURE_PATH "skybox/negx.jpg", TEXTURE_PATH "skybox/posx.jpg",
		TEXTURE_PATH "skybox/negy.jpg", TEXTURE_PATH "skybox/posy.jpg",
		TEXTURE_PATH "skybox/negz.jpg", TEXTURE_PATH "skybox/posz.jpg" };
	const char *skyboxBall[] = { TEXTURE_PATH "skyboxBall/negx.jpg", TEXTURE_PATH "skyboxBall/posx.jpg",
		TEXTURE_PATH "skyboxBall/negy.jpg", TEXTURE_PATH "skyboxBall/posy.jpg",
		TEXTURE_PATH "skyboxBall/negz.jpg", TEXTURE_PATH "skyboxBall/posz.jpg" };
	loadCubemap(skybox, sizeof(skybox) / sizeof(char*), textures["room"]);
	loadCubemap(skyboxBall, sizeof(skyboxBall) / sizeof(char*), textures["roomBall"]);
#endif

	createDepthBuffer();
	createSampler();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);
	
	return 0;
}

void GraphicsSubsystem::loadTextureUnits(const char *textureUnits[], int tsize)
{
	for (int i = 0; i < tsize; i++)
		texUnits[textureUnits[i]] = i;
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
		shadowTexUnit[i] = tsize + i;
}

void GraphicsSubsystem::loadUniforms(GLuint pr, const char *uniforms[], int usize, const char *blocks[], int bsize)
{
	for (int i = 0; i < usize; i++)
		programUniforms[pr][uniforms[i]] = glGetUniformLocation(pr, uniforms[i]);
	for (int i = 0; i < bsize; i++)
		programUniforms[pr][blocks[i]] = glGetUniformBlockIndex(pr, blocks[i]);
}

void GraphicsSubsystem::loadTexture(const char *filename, GLuint &texture)
{
	std::auto_ptr<glimg::ImageSet> pImageSet(glimg::loaders::stb::LoadFromFile(filename));

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glimg::SingleImage image = pImageSet->GetImage(0, 0, 0);
	glimg::Dimensions dims = image.GetDimensions();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dims.width, dims.height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image.GetImageData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pImageSet->GetMipmapCount() - 1);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GraphicsSubsystem::loadCubemap(const char *filenames[], int csize, GLuint &texture)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for(int i = 0; i < csize; i++)
	{
		std::auto_ptr<glimg::ImageSet> pImageSet(glimg::loaders::stb::LoadFromFile(filenames[i]));
		glimg::SingleImage image = pImageSet->GetImage(0, 0, 0);
		glimg::Dimensions dims = image.GetDimensions();

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,	GL_RGB, dims.width, dims.height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, image.GetImageData());
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GraphicsSubsystem::createDepthBuffer()
{
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++) 
	{
		glGenTextures(1, &shadowMapTextures[i]);
		glBindTexture(GL_TEXTURE_2D, shadowMapTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, windowSize.x, windowSize.y, 0, 
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenFramebuffers(1, &shadowFbo[i]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFbo[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTextures[i], 0);
		glDrawBuffer(GL_NONE);

		GLenum Status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE) {
			printf("FB error, status: 0x%x\n", Status);
			return;
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
}

void GraphicsSubsystem::reallocShadowTextures()
{
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++) 
	{
		glBindTexture(GL_TEXTURE_2D, shadowMapTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, windowSize.x, windowSize.y, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
}

void GraphicsSubsystem::createSampler()
{
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void GraphicsSubsystem::loadShaders()
{
	std::vector<shaderStringPair> shadow;
	shadow.push_back(std::make_pair(GL_VERTEX_SHADER, "data/shaders/shadow.glslv"));
	shaders["shadow"] = ShaderWorker::createProgramFromFiles(shadow);

	std::vector<shaderStringPair> simple;
	simple.push_back(std::make_pair(GL_VERTEX_SHADER, "data/shaders/simple.glslv"));
	simple.push_back(std::make_pair(GL_FRAGMENT_SHADER, "data/shaders/simple.glslf"));
	shaders["simple"] = ShaderWorker::createProgramFromFiles(simple);

	std::vector<shaderStringPair> skybox;
	skybox.push_back(std::make_pair(GL_VERTEX_SHADER, "data/shaders/skybox.glslv"));
	skybox.push_back(std::make_pair(GL_FRAGMENT_SHADER, "data/shaders/skybox.glslf"));
	shaders["skybox"] = ShaderWorker::createProgramFromFiles(skybox);

	std::vector<shaderStringPair> plane;
	plane.push_back(std::make_pair(GL_VERTEX_SHADER, "data/shaders/plane.glslv"));
	plane.push_back(std::make_pair(GL_FRAGMENT_SHADER, "data/shaders/plane.glslf"));
	shaders["plane"] = ShaderWorker::createProgramFromFiles(plane);

	std::vector<shaderStringPair> ball;
	ball.push_back(std::make_pair(GL_VERTEX_SHADER, "data/shaders/ball.glslv"));
	ball.push_back(std::make_pair(GL_FRAGMENT_SHADER, "data/shaders/ball.glslf"));
	shaders["ball"] = ShaderWorker::createProgramFromFiles(ball);
}


void GraphicsSubsystem::loadBuffers()
{
	glGenBuffers(1, &uniformBuffers["matrices"]);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["matrices"]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndexes["matrices"], uniformBuffers["matrices"], 0, sizeof(glm::mat4) * 2);

	glGenBuffers(1, &uniformBuffers["light"]);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["light"]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBlock), NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndexes["light"], uniformBuffers["light"], 0, sizeof(LightBlock));

	glGenBuffers(1, &uniformBuffers["material"]);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["material"]);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightBlock), NULL, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndexes["material"], uniformBuffers["material"], 0, sizeof(LightBlock));
}

void GraphicsSubsystem::loadUniforms()
{
	GLuint shadow = shaders["shadow"];
	GLuint simple = shaders["simple"];
	GLuint skybox = shaders["skybox"];
	GLuint plane = shaders["plane"];
	GLuint ball = shaders["ball"];
	
	programUniforms[shadow]["modelToClipMatrix"] = glGetUniformLocation(shadow, "modelToClipMatrix");
	
	const char *simpleUniforms[] = { "modelToWorldMatrix", "baseColor" };
	const char *simpleBlocks[] = { "GlobalMatrices" };
	loadUniforms(simple, simpleUniforms, sizeof(simpleUniforms) / sizeof(char*), simpleBlocks, sizeof(simpleBlocks) / sizeof(char*));

	const char *skyboxUniforms[] = { "modelToWorldMatrix", "skybox" };
	const char *skyboxBlocks[] = { "GlobalMatrices" };
	loadUniforms(skybox, skyboxUniforms, sizeof(skyboxUniforms) / sizeof(char*), skyboxBlocks, sizeof(skyboxBlocks) / sizeof(char*));

	const char *planeUniforms[] = { "modelToWorldMatrix", "normalModelToCameraMatrix", "modelToLightToClipMatrix",
		"textureScale", "colorTexture", "shadowTexture", "shadowTexSize" };
	const char *planeBlocks[] = { "GlobalMatrices", "Light", "Material" };
	loadUniforms(plane, planeUniforms, sizeof(planeUniforms) / sizeof(char*), planeBlocks, sizeof(planeBlocks) / sizeof(char*));

	const char *ballUniforms[] = { "modelToWorldMatrix", "normalModelToCameraMatrix", "normalModelToWorldMatrix",
		"worldToLightMatrix", "worldToLightITMatrix", "colorTexture", "skybox", "camPos" };
	const char *ballBlocks[] = { "GlobalMatrices", "Light", "Material" };
	loadUniforms(ball, ballUniforms, sizeof(ballUniforms) / sizeof(char*), ballBlocks, sizeof(ballBlocks) / sizeof(char*));

	glUniformBlockBinding(skybox, programUniforms[skybox]["GlobalMatrices"], bindingIndexes["matrices"]);

	glUniformBlockBinding(plane, programUniforms[plane]["GlobalMatrices"], bindingIndexes["matrices"]);
	glUniformBlockBinding(plane, programUniforms[plane]["Light"], bindingIndexes["light"]);
	glUniformBlockBinding(plane, programUniforms[plane]["Material"], bindingIndexes["material"]);

	glUniformBlockBinding(ball, programUniforms[ball]["GlobalMatrices"], bindingIndexes["matrices"]);
	glUniformBlockBinding(ball, programUniforms[ball]["Light"], bindingIndexes["light"]);
	glUniformBlockBinding(ball, programUniforms[ball]["Material"], bindingIndexes["material"]);

	glUseProgram(skybox);
	glUniform1i(programUniforms[skybox]["skybox"], texUnits["room"]);
	glUseProgram(0);

	glUseProgram(plane);
	glUniform1iv(programUniforms[plane]["shadowTexture"], NUMBER_OF_LIGHTS, shadowTexUnit);
	glUseProgram(0);

	glUseProgram(ball);
	glUniform1i(programUniforms[ball]["colorTexture"], texUnits["ball"]);
	glUniform1i(programUniforms[ball]["skybox"], texUnits["roomBall"]);
	glUseProgram(0);
}

glm::mat4 GraphicsSubsystem::calcLookAtMatrix(const glm::vec3 &cameraPt, const glm::vec3 &lookPt, const glm::vec3 &upPt)
{
	glm::vec3 lookDir = glm::normalize(lookPt - cameraPt);
	glm::vec3 upDir = glm::normalize(upPt);

	glm::vec3 rightDir = glm::normalize(glm::cross(lookDir, upDir));
	glm::vec3 perpUpDir = glm::cross(rightDir, lookDir);

	glm::mat4 rotMat(1.0f);
	rotMat[0] = glm::vec4(rightDir, 0.0f);
	rotMat[1] = glm::vec4(perpUpDir, 0.0f);
	rotMat[2] = glm::vec4(-lookDir, 0.0f);

	rotMat = glm::transpose(rotMat);

	glm::mat4 transMat(1.0f);
	transMat[3] = glm::vec4(-cameraPt, 1.0f);

	return rotMat * transMat;
}

glm::vec3 GraphicsSubsystem::resolveCamPosition()
{
	float phi = sphereCamRelPos.x / 180.0f * M_PI;
	float theta = (sphereCamRelPos.y) / 180.0f * M_PI;

	float fSinTheta = sinf(theta);
	float fCosTheta = cosf(theta);
	float fCosPhi = cosf(phi);
	float fSinPhi = sinf(phi);

	glm::vec3 dirToCamera(fSinTheta * fCosPhi, fCosTheta, fSinTheta * fSinPhi);
	viewVector = glm::normalize(-dirToCamera);
	return (dirToCamera * sphereCamRelPos.z) + camTarget;
}

glm::vec3 GraphicsSubsystem::getViewVector()
{
	return viewVector;
}

void GraphicsSubsystem::drawBall(const Sphere &ball)
{
	GLuint ballpr = shaders["ball"];
	glUseProgram(ballpr);

	glm::mat4 modelToWorld = ball.getModelToWorldMat();
	glm::mat3 normWorldMatrix = glm::mat3(glm::transpose(glm::inverse(modelToWorld)));
	glm::mat3 normCamMatrix = glm::mat3(glm::transpose(glm::inverse(worldToCam * modelToWorld)));

	glUniformMatrix4fv(programUniforms[ballpr]["modelToWorldMatrix"], 1, GL_FALSE, glm::value_ptr(modelToWorld));
	glUniformMatrix3fv(programUniforms[ballpr]["normalModelToWorldMatrix"], 1, GL_FALSE, glm::value_ptr(normWorldMatrix));
	glUniformMatrix3fv(programUniforms[ballpr]["normalModelToCameraMatrix"], 1, GL_FALSE, glm::value_ptr(normCamMatrix));

	glm::mat4 worldToLightMatrix = glm::scale(glm::mat4(1.0), glm::vec3(IBLscale)); 
	glm::mat3 worldToLightITMatrix = glm::mat3(glm::transpose(glm::inverse(worldToLightMatrix)));
	glUniformMatrix4fv(programUniforms[ballpr]["worldToLightMatrix"], 1, GL_FALSE, glm::value_ptr(worldToLightMatrix));
	glUniformMatrix3fv(programUniforms[ballpr]["worldToLightITMatrix"], 1, GL_FALSE, glm::value_ptr(worldToLightITMatrix));

	glUniform3f(programUniforms[ballpr]["camPos"], camPos.x, camPos.y, camPos.z);

	glActiveTexture(GL_TEXTURE0 + texUnits["roomBall"]);  
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures["roomBall"]);

	glActiveTexture(GL_TEXTURE0 + texUnits["ball"]);
	glBindTexture(GL_TEXTURE_2D, textures["ball"]);
	glBindSampler(texUnits["ball"], sampler);
	
	ball.draw();
	
	glBindSampler(texUnits["ball"], 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

std::string GraphicsSubsystem::getWoodTexture()
{
	return std::string("wood");
}

std::string GraphicsSubsystem::getClothTexture()
{
	return std::string("cloth");
}

void GraphicsSubsystem::drawPlane(const Plane &plane, const std::string &textureName)
{
	int planepr = shaders["plane"];
	glUseProgram(shaders["plane"]);

	glm::mat4 modelToWorld = plane.getModelToWorldMat();
	glm::mat3 normMatrix = glm::mat3(glm::transpose(glm::inverse(worldToCam * modelToWorld)));
	glUniformMatrix3fv(programUniforms[planepr]["normalModelToCameraMatrix"], 1, GL_FALSE, glm::value_ptr(normMatrix));

	glUniformMatrix4fv(programUniforms[planepr]["modelToWorldMatrix"], 1, GL_FALSE, glm::value_ptr(modelToWorld));
	glUniformMatrix4fv(programUniforms[planepr]["modelToLightToClipMatrix"], NUMBER_OF_LIGHTS, GL_FALSE, glm::value_ptr(modelLightWorldClip[0]));
	
	glm::vec2 textureScale = plane.getTextureScale();
	glUniform2f(programUniforms[planepr]["textureScale"], textureScale.x, textureScale.y);
	glUniform2f(programUniforms[planepr]["shadowTexSize"], windowSize.x, windowSize.y);

	glUniform1i(programUniforms[planepr]["colorTexture"], texUnits[textureName]);
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		glActiveTexture(GL_TEXTURE0 + shadowTexUnit[i]);  
		glBindTexture(GL_TEXTURE_2D, shadowMapTextures[i]);
	}

	glActiveTexture(GL_TEXTURE0 + texUnits[textureName]);
	glBindTexture(GL_TEXTURE_2D, textures[textureName]);
	glBindSampler(texUnits[textureName], sampler);
	
	plane.draw();

	glBindSampler(texUnits[textureName], 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glUseProgram(0);
}

void GraphicsSubsystem::drawLight(const Mesh *reference, LightSubsystem &lss)
{
	const float refScale = 0.2f;
	GLuint simplepr = shaders["simple"];
	glUseProgram(simplepr);
	LightBlock lblock = lss.getLightInformation(worldToCam);
	std::vector<glm::vec3> lPosData = lss.getLightWorldPosition();

	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		glm::mat4 modelToWorld = glm::scale(glm::translate(glm::mat4(1.0), lPosData[i]), glm::vec3(refScale));
		glUniformMatrix4fv(programUniforms[simplepr]["modelToWorldMatrix"], 1, GL_FALSE, glm::value_ptr(modelToWorld));
		glUniform4f(programUniforms[simplepr]["baseColor"], lblock.lights[i].lightIntensity.x, lblock.lights[i].lightIntensity.y, lblock.lights[i].lightIntensity.z, lblock.lights[i].lightIntensity.w);
		reference->draw();
	}
	glUseProgram(0);
}

void GraphicsSubsystem::drawSkybox(const Cube &cube)
{
	GLuint skyboxpr = shaders["skybox"];
	glCullFace(GL_FRONT);
	glUseProgram(skyboxpr);

	glm::mat4 modelToWorld = cube.getModelToWorldMat();

	glUniformMatrix4fv(programUniforms[skyboxpr]["modelToWorldMatrix"], 1, GL_FALSE, glm::value_ptr(modelToWorld));

	glActiveTexture(GL_TEXTURE0 + texUnits["room"]);  
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures["room"]);
	
	cube.draw();
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glUseProgram(0);
	glCullFace(GL_BACK);
}

void GraphicsSubsystem::shadowMapPass(const Mesh *target, LightSubsystem &lss)
{
	GLuint shadowpr = shaders["shadow"];
	glClearDepth(1.0f);
	glUseProgram(shadowpr);
	std::vector<glm::vec3> lPosData = lss.getLightWorldPosition();

	glm::mat4 modelMatrix = target->getModelToWorldMat();
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFbo[i]);
		glClear(GL_DEPTH_BUFFER_BIT);

		modelLightWorldClip[i] = glm::perspective(45.0f, (windowSize.x / (float)windowSize.y), zNear, zFar) *
			calcLookAtMatrix(lPosData[i], target->getWorldPos(), glm::vec3(0.0f, 0.0f, 1.0f)); // (0, 0, 1) - optimized for the ball

		glm::mat4 modelToClipMatrix = modelLightWorldClip[i] * modelMatrix;
		glUniformMatrix4fv(programUniforms[shadowpr]["modelToClipMatrix"], 1, GL_FALSE, glm::value_ptr(modelToClipMatrix));

		target->draw();
	}
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsSubsystem::bindLighting(LightSubsystem &lss)
{
	LightBlock lightData = lss.getLightInformation(worldToCam);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["light"]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightData), &lightData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GraphicsSubsystem::bindMaterial(const MaterialBlock &matData)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["material"]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matData), &matData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GraphicsSubsystem::clearBuffers()
{
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsSubsystem::setCam()
{
	camPos = resolveCamPosition();
	worldToCam = calcLookAtMatrix(camPos, camTarget, glm::vec3(0.0f, 1.0f, 0.0f));
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["matrices"]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(worldToCam));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GraphicsSubsystem::accumFrame(int cur, int n)
{
	if(cur == 0)
		glAccum(GL_LOAD, 1.0f / n);
	else
		glAccum(GL_ACCUM, 1.0f / n);
}

void GraphicsSubsystem::returnFrame()
{
	glAccum(GL_RETURN, 1.0);
}

void GraphicsSubsystem::swapBuffers()
{
	glutSwapBuffers();
}

void GraphicsSubsystem::reshape(int w, int h)
{	
	glm::mat4 persMatrix = glm::perspective(45.0f, (w / (float)h), zNear, zFar);

	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers["matrices"]);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(persMatrix));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	windowSize = glm::ivec2(w, h);
	reallocShadowTextures();
}

void GraphicsSubsystem::setCamTarget(const glm::vec3 &camt)
{
	camTarget = camt;
}

void GraphicsSubsystem::rotateCam(const glm::vec3 &diff)
{
	sphereCamRelPos += diff;
	sphereCamRelPos.y = glm::clamp(sphereCamRelPos.y, minCamAngle, maxCamAngle);
	sphereCamRelPos.z = glm::clamp(sphereCamRelPos.z, minCamDistance, maxCamDistance);
}

GraphicsSubsystem::~GraphicsSubsystem()
{
	for (std::map<std::string, GLuint>::iterator buff = uniformBuffers.begin(); buff != uniformBuffers.end(); buff++)
		glDeleteBuffers(1, &buff->second);
	for (std::map<std::string, GLuint>::iterator tex = textures.begin(); tex != textures.end(); tex++)
		glDeleteTextures(1, &tex->second);
	glDeleteBuffers(NUMBER_OF_LIGHTS, shadowFbo);
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
		glDeleteTextures(1, &shadowMapTextures[i]);

}