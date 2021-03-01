#ifndef SHADER_H
#define SHADER_H
#include "glr_inline.h"

#include <string>

namespace glr {

typedef enum{
	PATH_TO_CODE,
	RAW_CODE
} shaderLoadType;

class shader
{
public:
	// program ID
	unsigned int ID;
	std::string name;

	// Empty Constructor
	shader(){ID = -1; name = "";};

	shader(const shader &src);	

	// Constructor reads and builds shader
	shader(const char* vertexPath, const char* fragmentPath, shaderLoadType type=PATH_TO_CODE);
	// Constructor reads and builds shader and adds name
	shader(const char* vertexPath, const char* fragmentPath, const char* name, shaderLoadType type=PATH_TO_CODE);
	// activate shader
	void use() const;

	// set uniform functions (best to just do this manually)
	// in other words, don't bother with these unless you want to
	// take the time to implement all the possibilities yourself
	//
	// By calling any one of these, the shader is set to be
	// active
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setVec3(const std::string &name, float value[3]) const;
	void setVec4(const std::string &name, float value[4]) const;
	void setMat4(const std::string &name, float* value) const;

	void operator=(const shader &src);	

	// also removes shader from gpu mem
	~shader();

private:
	std::string vertexCode, fragmentCode;
	void compileProgram(const char* vertexPath, const char* fragmentPath, shaderLoadType type);
};
}

#ifndef GLRENDER_STATIC
    #include <glr/shader.cpp>
#endif

#endif
