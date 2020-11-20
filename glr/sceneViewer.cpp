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
}