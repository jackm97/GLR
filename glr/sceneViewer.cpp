#include <glr/sceneViewer.h>

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

namespace glr {

GLRENDER_INLINE void sceneViewer::init()
{
	if (!isInit){
		isInit = true;

		shaders.reserve(MAX_SHADER_COUNT);
		textures.reserve(MAX_TEXTURE_COUNT);

		// assign an empty texture for shapes without textures
		// used within the draw function to determine if
		// a texture should be used in the fragment shader
		if (!textureExist("empty"))
		{	
			texture emptyTexture("empty");
			textures.push_back(emptyTexture);
			emptyTexture.glRelease();
		}
	}
}

/*
*
*
CAMERA STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::useOrtho(float left, float right, float bottom, float top, float zNear, float zFar)
{
	proj = glm::ortho(left, right, bottom, top, zNear, zFar);
}

GLRENDER_INLINE void sceneViewer::usePerspective(float fovy, float aspect, float zNear, float zFar)
{
	proj = glm::perspective(fovy, aspect, zNear, zFar);
}

GLRENDER_INLINE void sceneViewer::addCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir)
{
	if (cameraExist(name)) return;

	camera newCam;

	newCam.name = name;
	newCam.pos = eyePos;
	newCam.dir = glm::normalize(lookDir);
	newCam.up = glm::normalize(upDir);

	cameraList.push_back(newCam);
}

GLRENDER_INLINE void sceneViewer::modifyCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir)
{
	if (!cameraExist(name)) return;

	camera* camPtr;
	for (int i=0; i < cameraList.size(); i++)
	{
		if (name == cameraList[i].name)
		{
			camPtr = &(cameraList[i]);
			break;
		}
	}

	camPtr->name = name;
	camPtr->pos = eyePos;
	camPtr->dir = glm::normalize(lookDir);
	camPtr->up = glm::normalize(upDir);
}

GLRENDER_INLINE void sceneViewer::useCamera(std::string name)
{
	if (!cameraExist(name)) return;

	for (int i=0; i < cameraList.size(); i++)
	{
		if (name == cameraList[i].name)
		{
			cameraIdx = i;
			return;
		}
	}
}

GLRENDER_INLINE bool sceneViewer::cameraExist(std::string name)
{
	for (int i=0; i < cameraList.size(); i++)
		if (name == cameraList[i].name) return true;

	return false;
}

/*
*
*
LIGHTING STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::setAmbientLight(glm::vec3 color, float I)
{
	ambientColor = color;
	ambientI = I;
}
GLRENDER_INLINE void sceneViewer::addDirLight(glm::vec3 dir, glm::vec3 color, float I, float spec)
{
	dirLight newLight;

	newLight.dir = dir;
	newLight.color = color;
	newLight.I = I;
	newLight.spec = spec;

	dirLightList.push_back(newLight);
}

GLRENDER_INLINE void sceneViewer::addPointLight(glm::vec3 pos, glm::vec3 color, float I, float spec)
{
	pointLight newLight;

	newLight.pos = pos;
	newLight.color = color;
	newLight.I = I;
	newLight.spec = spec;

	pointLightList.push_back(newLight);
}

GLRENDER_INLINE void sceneViewer::addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float I, float cutoffAngle, float spec)
{
	spotLight newLight;

	newLight.pos = pos;
	newLight.dir = dir;
	newLight.color = color;
	newLight.I = I;
	newLight.spec = spec;
	newLight.cutOffAngle = cutoffAngle;

	spotLightList.push_back(newLight);
}

/*
*
*
OTHER STUFF
*
*
*/

GLRENDER_INLINE void sceneViewer::setSceneTransform(const glm::mat4 m)
{
	model = m;
}

GLRENDER_INLINE void sceneViewer::drawScene()
{
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		drawObj(wavefrontObjList[obj]);
}

GLRENDER_INLINE void sceneViewer::setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr)
{

	// camera stuff
	camera* camPtr = &( cameraList[ cameraIdx ] );
	view = glm::lookAt(camPtr->pos, camPtr->pos + camPtr->dir, camPtr->up);
	unsigned int uLocation = glGetUniformLocation(shaderPtr->ID, "cameraPos");
	glUniform3fv(uLocation, 1, glm::value_ptr(camPtr->pos));

	// mvp matrix
	uLocation = glGetUniformLocation(shaderPtr->ID, "m");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(model * obj.modelMatrix));
	uLocation = glGetUniformLocation(shaderPtr->ID, "v");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(view));
	uLocation = glGetUniformLocation(shaderPtr->ID, "p");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(proj));
	
	// material info
	shaderPtr->setVec3("Ka",  mat.ambient);
	shaderPtr->setVec3("Kd",  mat.diffuse);
	shaderPtr->setVec3("Ks",  mat.specular);
	shaderPtr->setVec3("Ke",  mat.emission);
	shaderPtr->setFloat("Ns", mat.shininess);

	// ambient light
	uLocation = glGetUniformLocation(shaderPtr->ID, "ambientColor");
	glUniform3fv(uLocation, 1, glm::value_ptr(ambientColor));
	uLocation = glGetUniformLocation(shaderPtr->ID, "ambientI");
	glUniform1f(uLocation, ambientI);


	// directional lights
	std::vector<float> dirs;
	std::vector<float> colors;
	std::vector<float> Is;
	std::vector<float> specs;
	for (int l=0; l < dirLightList.size(); l++)
	{
		dirs.push_back(dirLightList[l].dir.x);
		dirs.push_back(dirLightList[l].dir.y);
		dirs.push_back(dirLightList[l].dir.z);
		
		colors.push_back(dirLightList[l].color.x);
		colors.push_back(dirLightList[l].color.y);
		colors.push_back(dirLightList[l].color.z);

		Is.push_back(dirLightList[l].I);
		specs.push_back(dirLightList[l].spec);
	}
	uLocation = glGetUniformLocation(shaderPtr->ID, "dirLightDir");
	glUniform3fv(uLocation, dirLightList.size(), dirs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "dirLightColor");
	glUniform3fv(uLocation, dirLightList.size(), colors.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "dirLightI");
	glUniform1fv(uLocation, dirLightList.size(), Is.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "dirLightSpec");
	glUniform1fv(uLocation, dirLightList.size(), specs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "numDirLight");
	glUniform1i(uLocation, dirLightList.size());

	// point lights
	std::vector<float> pos;
	colors.clear();
	Is.clear();
	specs.clear();
	for (int l=0; l < pointLightList.size(); l++)
	{
		pos.push_back(pointLightList[l].pos.x);
		pos.push_back(pointLightList[l].pos.y);
		pos.push_back(pointLightList[l].pos.z);
		
		colors.push_back(pointLightList[l].color.x);
		colors.push_back(pointLightList[l].color.y);
		colors.push_back(pointLightList[l].color.z);

		Is.push_back(pointLightList[l].I);
		specs.push_back(pointLightList[l].spec);
	}
	uLocation = glGetUniformLocation(shaderPtr->ID, "pointLightPos");
	glUniform3fv(uLocation, pointLightList.size(), pos.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "pointLightColor");
	glUniform3fv(uLocation, pointLightList.size(), colors.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "pointLightI");
	glUniform1fv(uLocation, pointLightList.size(), Is.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "pointLightSpec");
	glUniform1fv(uLocation, pointLightList.size(), specs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "numPointLight");
	glUniform1i(uLocation, pointLightList.size());

	// spot lights
	pos.clear();
	dirs.clear();
	colors.clear();
	Is.clear();
	specs.clear();
	std::vector<float> cutoffs;
	for (int l=0; l < spotLightList.size(); l++)
	{
		pos.push_back(spotLightList[l].pos.x);
		pos.push_back(spotLightList[l].pos.y);
		pos.push_back(spotLightList[l].pos.z);

		dirs.push_back(spotLightList[l].dir.x);
		dirs.push_back(spotLightList[l].dir.y);
		dirs.push_back(spotLightList[l].dir.z);
		
		colors.push_back(spotLightList[l].color.x);
		colors.push_back(spotLightList[l].color.y);
		colors.push_back(spotLightList[l].color.z);

		Is.push_back(spotLightList[l].I);
		specs.push_back(spotLightList[l].spec);

		cutoffs.push_back(spotLightList[l].cutOffAngle);
	}
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightPos");
	glUniform3fv(uLocation, spotLightList.size(), pos.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightDir");
	glUniform3fv(uLocation, spotLightList.size(), dirs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightColor");
	glUniform3fv(uLocation, spotLightList.size(), colors.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightI");
	glUniform1fv(uLocation, spotLightList.size(), Is.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightSpec");
	glUniform1fv(uLocation, spotLightList.size(), specs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "spotLightCutoff");
	glUniform1fv(uLocation, spotLightList.size(), cutoffs.data());
	uLocation = glGetUniformLocation(shaderPtr->ID, "numSpotLight");
	glUniform1i(uLocation, spotLightList.size());

	uLocation = glGetUniformLocation(shaderPtr->ID, "shapeCenter");
	glUniform3fv(uLocation, 1, glm::value_ptr(obj.shapeCenters[shapeIdx]));
}

} // namespace glr