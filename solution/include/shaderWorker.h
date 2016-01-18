#ifndef __SHADER_WORKER_H
#define __SHADER_WORKER_H

#include <vector>
#include <string>
#include <GL/glew.h>

#define shaderStringPair std::pair<GLenum, std::string>

class ShaderWorker
{
public:
	static GLuint createShader(GLenum eShaderType, const std::string &strShaderFile);
	static GLuint createProgramFromShaders(const std::vector<GLuint> &shaderList);
	static GLuint createProgramFromFiles(const std::vector<shaderStringPair> &filePathList);
private:
	static GLuint loadShaders(const std::vector<shaderStringPair> &vshader);
	static int loadShaderFromFile(const std::string &filePath, std::string &shaderOut);
};

#endif