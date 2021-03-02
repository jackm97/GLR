#include <glr/shader.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <fstream>
#include <sstream>
#include <iostream>

namespace glr {

GLRENDER_INLINE shader::shader(const char* vertexPath, const char* fragmentPath, shaderLoadType type)
{
	this->name = "";
	compileProgram(vertexPath, fragmentPath, type);
}


	// Constructor reads and builds shader and adds name
GLRENDER_INLINE shader::shader(const char* vertexPath, const char* fragmentPath, const char* name, shaderLoadType type)
{
    // gladLoadGL();
	this->name = name;
	compileProgram(vertexPath, fragmentPath, type);
}

GLRENDER_INLINE shader::shader(const shader &src)
{
	*this = src;
}

GLRENDER_INLINE void shader::use() const
{
	glUseProgram(ID);
}

GLRENDER_INLINE void shader::setBool(const std::string &name, bool value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uLocation, (int) value);
}

GLRENDER_INLINE void shader::setInt(const std::string &name, int value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniform1i(uLocation, value);
}

GLRENDER_INLINE void shader::setFloat(const std::string &name, float value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniform1f(uLocation, value);
}

GLRENDER_INLINE void shader::setVec3(const std::string &name, float value[3]) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniform3f(uLocation, value[0], value[1], value[2]);
}

GLRENDER_INLINE void shader::setVec4(const std::string &name, float value[4]) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniform4f(uLocation, value[0], value[1], value[2], value[3]);
}

GLRENDER_INLINE void shader::setMat4(const std::string &name, float* value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, value);
}

GLRENDER_INLINE void shader::operator=(const shader &src)
{
	name = src.name;
	vertexCode = src.vertexCode;
	fragmentCode = src.fragmentCode;

	compileProgram(vertexCode.c_str(), fragmentCode.c_str(), RAW_CODE);
}

GLRENDER_INLINE shader::~shader()
{
	glDeleteProgram(this->ID);
}

// PRIVATE
GLRENDER_INLINE void shader::compileProgram(const char* vertexPath, const char* fragmentPath, shaderLoadType type)
{	
	//1. retrieve vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	if (type == PATH_TO_CODE) 
	{
		// ensure ifstream objects can throw exceptions
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			//open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream, colormapStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			
			vShaderFile.close();
			fShaderFile.close();
			
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch(std::ifstream::failure e)
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			exit(1);
		}
	}
	else if (type = RAW_CODE)
	{
		vertexCode = vertexPath;
		fragmentCode = fragmentPath;
	}

	this->vertexCode = vertexCode;
	this->fragmentCode = fragmentCode;
	
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	
	// 2. Compile Shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(1);
	}

	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(1);
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	//print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		exit(1);
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
}