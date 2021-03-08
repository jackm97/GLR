#include <glr/shader.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <fstream>
#include <sstream>
#include <iostream>

namespace glr {

GLRENDER_INLINE shader::shader(const char* vertex_shader_info, const char* fragment_shader_info, shaderLoadType type)
{
	this->name_ = "";
	compileProgram(vertex_shader_info, fragment_shader_info, type);
}


	// Constructor reads and builds shader and adds name
GLRENDER_INLINE shader::shader(const char* vertex_shader_info, const char* fragment_shader_info, const char* name, shaderLoadType type)
{
    // gladLoadGL();
	this->name_ = name;
	compileProgram(vertex_shader_info, fragment_shader_info, type);
}

GLRENDER_INLINE shader::shader(const shader &src)
{
	*this = src;
}

GLRENDER_INLINE void shader::use() const
{
	glUseProgram(ID_);
}

GLRENDER_INLINE void shader::setBool(const std::string &name, bool value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniform1i(uLocation, (int) value);
}

GLRENDER_INLINE void shader::setInt(const std::string &name, int value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniform1i(uLocation, value);
}

GLRENDER_INLINE void shader::setFloat(const std::string &name, float value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniform1f(uLocation, value);
}

GLRENDER_INLINE void shader::setVec3(const std::string &name, float value[3]) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniform3f(uLocation, value[0], value[1], value[2]);
}

GLRENDER_INLINE void shader::setVec4(const std::string &name, float value[4]) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniform4f(uLocation, value[0], value[1], value[2], value[3]);
}

GLRENDER_INLINE void shader::setMat4(const std::string &name, float* value) const
{
	this->use();
	unsigned int uLocation = glGetUniformLocation(ID_, name.c_str());
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, value);
}

GLRENDER_INLINE void shader::operator=(const shader &src)
{
	name_ = src.name_;
	vertex_code_ = src.vertex_code_;
	fragment_code_ = src.fragment_code_;

	if (ID_ != -1)
		glRelease();
	compileProgram(vertex_code_.c_str(), fragment_code_.c_str(), RAW_CODE);
}

GLRENDER_INLINE void shader::glRelease()
{
	if (this->ID_ == -1)
		return;
	glDeleteProgram(this->ID_);
	this->ID_ = -1;
}

GLRENDER_INLINE shader::~shader()
{
	glRelease();
}

// PRIVATE
GLRENDER_INLINE void shader::compileProgram(const char* vertex_shader_info, const char* fragment_shader_info, shaderLoadType type)
{	
	//1. retrieve vertex/fragment source code from filePath
	std::string vertex_code;
	std::string fragment_code;
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
			vShaderFile.open(vertex_shader_info);
			fShaderFile.open(fragment_shader_info);
			std::stringstream vShaderStream, fShaderStream, colormapStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			
			vShaderFile.close();
			fShaderFile.close();
			
			vertex_code = vShaderStream.str();
			fragment_code = fShaderStream.str();
		}
		catch(std::ifstream::failure e)
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
			exit(1);
		}
	}
	else if (type = RAW_CODE)
	{
		vertex_code = vertex_shader_info;
		fragment_code = fragment_shader_info;
	}

	this->vertex_code_ = vertex_code;
	this->fragment_code_ = fragment_code;
	
	const char* v_shader_code = vertex_code_.c_str();
	const char* f_shader_code = fragment_code_.c_str();
	
	// 2. Compile Shaders
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &v_shader_code, NULL);
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
	glShaderSource(fragment, 1, &f_shader_code, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(1);
	}

	ID_ = glCreateProgram();
	glAttachShader(ID_, vertex);
	glAttachShader(ID_, fragment);
	glLinkProgram(ID_);
	//print linking errors if any
	glGetProgramiv(ID_, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID_, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		exit(1);
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
}