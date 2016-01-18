#include "shaderWorker.h"

#include <iostream>
#include <fstream>
#include <algorithm>

GLuint ShaderWorker::createShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch (eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}
		std::cerr << "Compile failure in " << strShaderType << " shader" << std::endl << strInfoLog << std::endl;
		delete[] strInfoLog;
	}

	return shader;
}

GLuint ShaderWorker::createProgramFromShaders(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		std::cerr << "Linker failure: " << strInfoLog << std::endl;
		delete[] strInfoLog;
	}

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}


GLuint ShaderWorker::createProgramFromFiles(const std::vector<shaderStringPair> &filePathList)
{
	std::vector<shaderStringPair> vec;
	for (std::vector<shaderStringPair>::const_iterator it=filePathList.begin(); it != filePathList.end(); it++)
	{
		std::string shaderProgram;
		if (!loadShaderFromFile(it->second, shaderProgram))
			vec.push_back(std::make_pair(it->first, shaderProgram));
	}
	return loadShaders(vec);
}

GLuint ShaderWorker::loadShaders(const std::vector<shaderStringPair> &vshader)
{
	std::vector<GLuint> myshaderList;
	for (std::vector<shaderStringPair>::const_iterator it=vshader.begin(); it != vshader.end(); it++)
		myshaderList.push_back(createShader(it->first, it->second));
	GLuint myProgram = createProgramFromShaders(myshaderList);
	std::for_each(myshaderList.begin(), myshaderList.end(), glDeleteShader);
	return myProgram;
}


int ShaderWorker::loadShaderFromFile(const std::string &filePath, std::string &shaderOut)
{
	std::string shader = "";
	std::ifstream is(filePath);
	if (is) {
		std::string tmpstr;
		while (!is.eof()) {
			std::getline(is, tmpstr);
			shader += tmpstr + "\n";
		}
		is.close();
	}
	else {
		std::cout << "error: only " << is.gcount() << " can be read\n";
		return 1;
	}
	shaderOut = shader;

	return 0;
}

