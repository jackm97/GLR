#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H
#include "glr_inline.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glr/shader.h>
#include <glr/texture.h>

#define TINYOBJ_CUSTOM_NAMESPACE glr
#include <glr/tinyobjloader/tiny_obj_loader.h>

#include <string>
#include <vector>

namespace glr {

struct wavefrontObj;
struct camera;
struct dirLight;
struct pointLight;
struct spotLight;

class sceneViewer
{
    public:

        sceneViewer(){};

        void addWavefront(std::string objPath, std::string baseDir, std::string name);

        void initGLBuffers(bool calcNormals=false, bool flipNormals=false);

        void initGLBuffers(std::string objName, bool calcNormals=false, bool flipNormals=false);

        
        // shader stuff
        
        void addShader(std::string vertPath, std::string fragPath, std::string shaderName);

        void useShaderForAll(std::string shaderName);

        void useShaderForObject(std::string objName, std::string shaderName);

        void useShaderForObjectList(std::vector<std::string> objectList, std::string shaderName);

        void useShaderForShape(std::string objName, std::string shapeName, std::string shaderName);

        void useShaderForShapeList(std::string objName, std::vector<std::string> shapeList, std::string shaderName);

        unsigned int getShaderID(std::string shaderName);

        bool shaderExist(std::string shaderName);

        
        // texture stuff
        
        void addtexture(std::string texturePath, std::string textureName);
        
        void addTexture(int width, int height, std::string textureName);
        
        void uploadPix2Tex(std::string textureName, GLenum format, GLenum type, void* data);

        void usetextureForAll(std::string textureName);

        void usetextureForObject(std::string objName, std::string textureName);

        void usetextureForObjectList(std::vector<std::string> objectList, std::string textureName);

        void usetextureForShape(std::string objName, std::string shapeName, std::string textureName);

        void usetextureForShapeList(std::string objName, std::vector<std::string> shapeList, std::string textureName);

        unsigned int gettextureID(std::string textureName);

        bool textureExist(std::string textureName);

        // Camera Stuff
        void useOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

        void usePerspective(float fovy, float aspect, float zNear, float zFar);

        void addCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir);

        void modifyCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir);

        void useCamera(std::string name);

        bool cameraExist(std::string name);

        // Lighting Stuff
        void setAmbientLight(glm::vec3 color, float I);
        
        void addDirLight(glm::vec3 dir, glm::vec3 color, float I, float spec = 1.);
        
        void addPointLight(glm::vec3 pos, glm::vec3 color, float I, float spec = 1.);
        
        void addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float I, float cutoffAngle, float spec = 1.);

        
        // other stuff
        void listShapes(std::string objName);

        void setObjTransfrom(std::string objName, glm::mat4 modelMatrix);
        
        void setSceneTransform(const glm::mat4 m);

        void drawScene();

        ~sceneViewer(){}

    private:
        std::vector<wavefrontObj> wavefrontObjList;
        glm::mat4 model{1.0f}; // (NOTE: model matrix is for whole scene)
        glm::mat4 view{1.0f}; //
        glm::mat4 proj{1.0f};

        std::vector<camera> cameraList; 
        unsigned int cameraIdx;

        glm::vec3 ambientColor{0.f};
        float ambientI{0.f};
        std::vector<dirLight> dirLightList;
        std::vector<pointLight> pointLightList;
        std::vector<spotLight> spotLightList;
        std::vector<shader> shaders;            
        std::vector<texture> textures;

        void initGLBuffers(wavefrontObj &obj, bool calcNormals=false, bool flipNormals=false);

        void useShaderForShape(wavefrontObj &obj, std::string shapeName, std::string shaderName);
        void useShaderForObject(wavefrontObj &obj, std::string shaderName);

        void usetextureForShape(wavefrontObj &obj, std::string shapeName, std::string textureName);
        void usetextureForObject(wavefrontObj &obj, std::string textureName);

        void drawObj(wavefrontObj &obj);

        void setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, unsigned int shaderIdx);

        glm::vec3 getShapeCenter(wavefrontObj &obj, unsigned int shapeIdx);
};

class wavefrontObj
{
    public:
        std::string objPath = "", baseDir = "";

        std::string name = "";
        
        std::vector<unsigned int> VAOList;
        
        tinyobj::attrib_t attrib; // see tinyobj docs
        std::vector<tinyobj::shape_t> shapes; // see tinyobj docs
        std::vector<tinyobj::material_t> materials; // see tinyobj docs

        // This vector has same size as shapes
        // and each entry is in index that points
        // to a shader in shaders.
        // shapes[i] will be rendered using
        // shaders[shaderIndices[i]]
        std::vector<int> shaderIndices;

        // works similar to shaderIndices
        std::vector<int> textureIndices;

        glm::mat4 modelMatrix{1.0f};

        std::vector<glm::vec3> shapeCenters;

        bool isLoadedIntoGL = false;

        friend sceneViewer;

    private:
        std::vector<bool> noUVMap;
};

struct camera
{
    public:
        std::string name;

        glm::vec3 pos;

        glm::vec3 dir;

        glm::vec3 up;
};

struct dirLight
{
    public:
        glm::vec3 dir;

        glm::vec3 color;

        float I;

        float spec;
};

struct pointLight
{
    public:
        glm::vec3 pos;

        glm::vec3 color;

        float I;

        float spec;
};

struct spotLight
{
    public:
        glm::vec3 pos;

        glm::vec3 dir;

        glm::vec3 color;

        float I;

        float spec;

        float cutOffAngle;
};
}

#ifndef GLRENDER_STATIC   
    #include <glr/sceneViewer.cpp>
#endif

#endif