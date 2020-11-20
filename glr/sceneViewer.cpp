#include <glr/sceneViewer.h>

#define CUSTOM_TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_INLINE GLRENDER_INLINE
#include <glr/tinyobjloader/tiny_obj_loader.h>

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

namespace glr {

GLRENDER_INLINE void sceneViewer::addWavefront(std::string objPath, std::string baseDir, std::string name)
{
	wavefrontObj newObj;

	newObj.name = name;

	newObj.objPath = objPath;
	if (baseDir[baseDir.length() - 1] != '/') baseDir += "/";
	newObj.baseDir = baseDir;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&newObj.attrib, &newObj.shapes, &newObj.materials, &warn, &err, objPath.c_str(), baseDir.c_str());

    if (!warn.empty()) {
    std::cout << warn << std::endl;
    }

    if (!err.empty()) {
    std::cerr << err << std::endl;
    }

    if (!ret) {
    exit(1);
    }

    newObj.shaderPtrs.resize(newObj.shapes.size());
	newObj.texturePtrs.resize(newObj.shapes.size());

	newObj.noUVMap.resize(newObj.shapes.size());

	wavefrontObjList.push_back(newObj);
}

GLRENDER_INLINE void sceneViewer::initGLBuffers(bool calcNormals, bool flipNormals)
{
    // gladLoadGL();
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		initGLBuffers(wavefrontObjList[obj], calcNormals, flipNormals);
}

GLRENDER_INLINE void sceneViewer::initGLBuffers(std::string objName, bool calcNormals, bool flipNormals)
{
    // gladLoadGL();
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name)
		{
			initGLBuffers(wavefrontObjList[obj], calcNormals, flipNormals);
		}
	}
}

GLRENDER_INLINE void sceneViewer::initGLBuffers(wavefrontObj &obj, bool calcNormals, bool flipNormals)
{
	if (obj.isLoadedIntoGL) return;

	// assign an empty texture for shapes without textures
	// used within the draw function to determine if
	// a texture should be used in the fragment shader
	if (!textureExist("empty"))
	{	
		texture emptyTexture("empty");
		textures.push_back(emptyTexture);
		emptyTexture.glRelease();
	}

	std::string baseDir = obj.baseDir;
	std::vector<unsigned int> &VAOList = obj.VAOList;
    
    tinyobj::attrib_t &attrib = obj.attrib;
    std::vector<tinyobj::shape_t> &shapes = obj.shapes;
    std::vector<tinyobj::material_t> &materials = obj.materials;
    
	int shapeNum = shapes.size();
	std::vector<float> vertexData;
	VAOList.resize(shapeNum);
	unsigned int VBO;

	// generate vertex arrays
	glGenVertexArrays(shapeNum, VAOList.data());
	
	// loop over shapes
	size_t vCount = 0;
	for (int s = 0; s < shapeNum; s++)
	{
		vertexData.clear();
		vCount = 0;
		
        // loop over faces
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			if (shapes[s].mesh.num_face_vertices[f] != 3)
				continue;
			
			// add texture if there is one (repeated textures are not added again)
			std::string diffuse_texname = materials[shapes[s].mesh.material_ids[0]].diffuse_texname;
			if (diffuse_texname.length() != 0)
			{
				addtexture(baseDir + diffuse_texname, diffuse_texname);
				usetextureForShape(obj, shapes[s].name, diffuse_texname);
			}
			else
			{
				usetextureForShape(obj, shapes[s].name, "empty");
			}
			
			// Loop over vertices in the face.
			for (size_t v = 0; v < 3; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
				tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
				tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];

                tinyobj::real_t nx;
                tinyobj::real_t ny;
                tinyobj::real_t nz;
				if (idx.normal_index!=-1)
				{
					nx = ((int) flipNormals * -2 + 1) * attrib.normals[3*idx.normal_index+0];
					ny = ((int) flipNormals * -2 + 1) * attrib.normals[3*idx.normal_index+1];
					nz = ((int) flipNormals * -2 + 1) * attrib.normals[3*idx.normal_index+2];
				}
				else
				{
					calcNormals = true;
				}
				
				tinyobj::real_t tx;
				tinyobj::real_t ty;
				if (idx.texcoord_index!=-1)
				{
					tx = attrib.texcoords[2*idx.texcoord_index+0];
					ty = attrib.texcoords[2*idx.texcoord_index+1];
				}
				else
				{
					tx = 0;
					ty = 0;
					obj.noUVMap[s] = true;
				}
				
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
				vertexData.push_back(vx);
				vertexData.push_back(vy);
				vertexData.push_back(vz);
				vertexData.push_back(nx);
				vertexData.push_back(ny);
				vertexData.push_back(nz);
				vertexData.push_back(tx);
				vertexData.push_back(ty);

				vCount++;
			}
			if (calcNormals)
			{
				glm::vec3 v3(vertexData[8*(vCount-3) + 0], vertexData[8*(vCount-3) + 1], vertexData[8*(vCount-3) + 2]);
				glm::vec3 v2(vertexData[8*(vCount-2) + 0], vertexData[8*(vCount-2) + 1], vertexData[8*(vCount-2) + 2]);
				glm::vec3 v1(vertexData[8*(vCount-1) + 0], vertexData[8*(vCount-1) + 1], vertexData[8*(vCount-1) + 2]);

				glm::vec3 v12 = v2 - v1;
				glm::vec3 v13 = v3 - v2;
				glm::vec3 n = glm::normalize(glm::cross(v13,v12));
				n *= ((int) flipNormals * -2 + 1); 
				(vertexData[8*(vCount-3) + 3 + 0]) = n.x; (vertexData[8*(vCount-3) + 3 + 1]) = n.y; (vertexData[8*(vCount-3) + 3 + 2]) = n.z; 
				(vertexData[8*(vCount-2) + 3 + 0]) = n.x; (vertexData[8*(vCount-2) + 3 + 1]) = n.y; (vertexData[8*(vCount-2) + 3 + 2]) = n.z; 
				(vertexData[8*(vCount-1) + 3 + 0]) = n.x; (vertexData[8*(vCount-1) + 3 + 1]) = n.y; (vertexData[8*(vCount-1) + 3 + 2]) = n.z; 
			}
			if (flipNormals)
			{

			}
			index_offset += 3;
		}

		obj.shapeCenters.push_back(getShapeCenter(obj, s));
		
		glBindVertexArray(VAOList[s]);
		
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_DYNAMIC_DRAW);
		
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
		glEnableVertexAttribArray(1);
		// uv coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6* sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	obj.isLoadedIntoGL = true;
}


/*
*
*
PUBLIC SHADER STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::addShader(std::string vertPath, std::string fragPath, std::string shaderName)
{
	if (shaderExist(shaderName)) return;
    shaders.push_back( shader( vertPath.c_str(), fragPath.c_str(), shaderName.c_str() ) );
}

GLRENDER_INLINE void sceneViewer::useShaderForAll(std::string shaderName)
{
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		useShaderForObject(wavefrontObjList[obj], shaderName);
}

GLRENDER_INLINE void sceneViewer::useShaderForObject(std::string objName, std::string shaderName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}
	
	for (int s = 0; s < wavefrontObjList[obj].shapes.size(); s++)
		useShaderForShape(wavefrontObjList[obj], wavefrontObjList[obj].shapes[s].name, shaderName);
}

GLRENDER_INLINE void sceneViewer::useShaderForObjectList(std::vector<std::string> objList, std::string shaderName)
{
	int obj;
	for (obj = 0; obj < objList.size(); obj++)
		useShaderForObject(objList[obj], shaderName);
}

GLRENDER_INLINE void sceneViewer::useShaderForShape(std::string objName, std::string shapeName, std::string shaderName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}

	useShaderForShape(wavefrontObjList[obj], shapeName, shaderName);
}

GLRENDER_INLINE void sceneViewer::useShaderForShapeList(std::string objName, std::vector<std::string> shapeList, std::string shaderName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}
	
	for (int s = 0; s < shapeList.size(); s++)
		useShaderForShape(wavefrontObjList[obj], shapeList[s], shaderName);
}

GLRENDER_INLINE unsigned int sceneViewer::getShaderID(std::string shaderName)
{
	int ID;
	for (int i=0; i<shaders.size(); i++)
		if (shaderName == shaders[i].name){ID = shaders[i].ID; break;}

	return ID;
}

GLRENDER_INLINE bool sceneViewer::shaderExist(std::string shaderName)
{
	for (int i=0; i<shaders.size(); i++)
		if (shaderName == shaders[i].name) return true;

	return false;
}

/*
*
*
PRIVATE SHADER STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::useShaderForObject(wavefrontObj &obj, std::string shaderName)
{
	for (int s = 0; s < obj.shapes.size(); s++)
		useShaderForShape(obj, obj.shapes[s].name, shaderName);
}

GLRENDER_INLINE void sceneViewer::useShaderForShape(wavefrontObj &obj, std::string shapeName, std::string shaderName)
{
	int s;
	for (s = 0; s < obj.shapes.size(); s++)
		if (shapeName == obj.shapes[s].name) break;

	int shad;
	for (shad = 0; shad < shaders.size(); shad++)
		if (shaderName == shaders[shad].name) break;

	obj.shaderPtrs[s] = &shaders[shad];
}

/*
*
*
PUBLIC TEXTURE STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::addtexture(std::string texturePath, std::string textureName)
{
	if (textureExist(textureName)) return;
	texture newTexture( texturePath, textureName );
	textures.push_back( newTexture );
	newTexture.glRelease();
}

GLRENDER_INLINE void sceneViewer::addTexture(int width, int height, std::string textureName)
{
    texture newTexture(width,height,textureName);
    textures.push_back( newTexture );
    newTexture.glRelease();
}

GLRENDER_INLINE void sceneViewer::uploadPix2Tex(std::string textureName, GLenum format, GLenum type, void* data)
{
    for (int t=0; t < textures.size(); t++)
    {
        if (textures[t].name == textureName)
            textures[t].loadPixels(format, type, data);
    }
}

GLRENDER_INLINE void sceneViewer::usetextureForAll(std::string textureName)
{
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		usetextureForObject(wavefrontObjList[obj], textureName);
}

GLRENDER_INLINE void sceneViewer::usetextureForObject(std::string objName, std::string textureName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}
	
	for (int s = 0; s < wavefrontObjList[obj].shapes.size(); s++)
		usetextureForShape(wavefrontObjList[obj], wavefrontObjList[obj].shapes[s].name, textureName);
}

GLRENDER_INLINE void sceneViewer::usetextureForObjectList(std::vector<std::string> objList, std::string textureName)
{
	int obj;
	for (obj = 0; obj < objList.size(); obj++)
		usetextureForObject(objList[obj], textureName);
}

GLRENDER_INLINE void sceneViewer::usetextureForShape(std::string objName, std::string shapeName, std::string textureName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}

	usetextureForShape(wavefrontObjList[obj], shapeName, textureName);
}

GLRENDER_INLINE void sceneViewer::usetextureForShapeList(std::string objName, std::vector<std::string> shapeList, std::string textureName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
	{
		if (objName == wavefrontObjList[obj].name) break;
	}
	
	for (int s = 0; s < shapeList.size(); s++)
		usetextureForShape(wavefrontObjList[obj], shapeList[s], textureName);
}

GLRENDER_INLINE unsigned int sceneViewer::gettextureID(std::string textureName)
{
	int ID;
	for (int i=0; i<textures.size(); i++)
		if (textureName == textures[i].name){ID = textures[i].ID; break;}

	return ID;
}

GLRENDER_INLINE bool sceneViewer::textureExist(std::string textureName)
{
	for (int i=0; i<textures.size(); i++)
		if (textureName == textures[i].name) return true;

	return false;
}

/*
*
*
PRIVATE TEXTURE STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::usetextureForObject(wavefrontObj &obj, std::string textureName)
{
	for (int s = 0; s < obj.shapes.size(); s++)
		usetextureForShape(obj, obj.shapes[s].name, textureName);
}

GLRENDER_INLINE void sceneViewer::usetextureForShape(wavefrontObj &obj, std::string shapeName, std::string textureName)
{
	int s;
	for (s = 0; s < obj.shapes.size(); s++)
		if (shapeName == obj.shapes[s].name) break;

	int tex;
	for (tex = 0; tex < textures.size(); tex++)
		if (textureName == textures[tex].name) break;

	obj.texturePtrs[s] = &textures[tex];
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
GLRENDER_INLINE void sceneViewer::listShapes(std::string objName)
{
	int obj;
	for (obj = 0; obj < wavefrontObjList.size(); obj++)
		if (objName == wavefrontObjList[obj].name) break;

	for (int s=0; s < wavefrontObjList[obj].shapes.size(); s++)
		std::cout << wavefrontObjList[obj].shapes[s].name << std::endl;
}

GLRENDER_INLINE void sceneViewer::setObjTransfrom(std::string objName, glm::mat4 modelMatrix)
{
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		if (objName == wavefrontObjList[obj].name)
		{
			wavefrontObjList[obj].modelMatrix = modelMatrix;
			return;
		}
}

GLRENDER_INLINE void sceneViewer::setSceneTransform(const glm::mat4 m)
{
	model = m;
}

GLRENDER_INLINE void sceneViewer::drawScene()
{
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		drawObj(wavefrontObjList[obj]);
}

GLRENDER_INLINE void sceneViewer::drawObj(wavefrontObj &obj)
{	
	std::vector<unsigned int> &VAOList = obj.VAOList;
    
    tinyobj::attrib_t &attrib = obj.attrib;
    std::vector<tinyobj::shape_t> &shapes = obj.shapes;
    std::vector<tinyobj::material_t> &materials = obj.materials;

    std::vector<shader*> &shaderPtrs = obj.shaderPtrs;

    std::vector<texture*> &texturePtrs = obj.texturePtrs;

	int shapeNum = VAOList.size();
	for (int s=0; s < shapeNum; s++)
	{
		// skip meshes with no faces (i.e. a point)
		if (shapes[s].mesh.num_face_vertices.size() == 0) 
			continue;
		
		//enable shader
		shaderPtrs[s]->use();

		// enable texture if there is one and if a uvmap exists
		texturePtrs[s]->bind();
		int textureAssigned = (texturePtrs[s]->name == "empty") ? 0 : 1;
		// if the user tried to assign a texture to a shape
		// with no UVMap, "empty" texture is assigned
		// if they texture map in a shader, they will
		// get white
		if (obj.noUVMap[s]) textures[0].bind();
		glUniform1i(glGetUniformLocation(shaderPtrs[s]->ID,"textureAssigned"), textureAssigned); 
		
		
		// set up uniforms
		tinyobj::material_t mat = materials[shapes[s].mesh.material_ids[0]];		
		setUniforms(obj, s, mat, shaderPtrs[s]);
		
		// draw
		glBindVertexArray(VAOList[s]);
		glDrawArrays(GL_TRIANGLES, 0, 3*shapes[s].mesh.num_face_vertices.size());
		glBindVertexArray(0);
	}
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

GLRENDER_INLINE glm::vec3 sceneViewer::getShapeCenter(wavefrontObj &obj, unsigned int shapeIdx)
{
	tinyobj::shape_t shape = obj.shapes[shapeIdx];
	tinyobj::attrib_t attrib = obj.attrib;
	// loop over faces
	size_t index_offset = 0;
	float minx, maxx, miny, maxy, minz, maxz;
	bool firstPoint = true;
	for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
		if (shape.mesh.num_face_vertices[f] != 3)
			continue;
		
		// Loop over vertices in the face.
		for (size_t v = 0; v < 3; v++) {
			// access to vertex
			tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
			tinyobj::real_t vx = attrib.vertices[3*idx.vertex_index+0];
			tinyobj::real_t vy = attrib.vertices[3*idx.vertex_index+1];
			tinyobj::real_t vz = attrib.vertices[3*idx.vertex_index+2];
			if (firstPoint)
			{
				minx = vx;
				maxx = vx;
				miny = vy;
				maxy = vy;
				minz = vz;
				maxz = vz;
				firstPoint = false;
			}
			else
			{
				minx = (vx < minx) ? vx : minx;
				maxx = (vx > maxx) ? vx : maxx;
				miny = (vy < miny) ? vy : miny;
				maxy = (vy > maxy) ? vy : maxy;
				minz = (vz < minz) ? vz : minz;
				maxz = (vz > maxz) ? vz : maxz;
			}			
		}
		index_offset += 3;
	}
	return glm::vec3{(maxx+minx)/2., (maxy+miny)/2., (maxz+minz)/2.};	
}
}