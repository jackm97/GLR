#ifndef RENDERBASE_H
#define RENDERBASE_H
#include "glr_inline.h"

#include <glr/shader.h>
#include <glr/texture.h>

#include <string>
#include <vector>

namespace glr {

class renderBase
{
    public:

        renderBase(){};

        virtual void init() = 0;

        void addWavefront(std::string objPath, std::string baseDir, std::string name, bool calcNormals=false, bool flipNormals=false);

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

        void deleteShader(std::string shaderName);

        
        // texture stuff
        
        void addTexture(std::string texturePath, std::string textureName);
        
        void addTexture(int width, int height, std::string textureName);
        
        void uploadPix2Tex(std::string textureName, GLenum format, GLenum type, void* data);

        void useTextureForAll(std::string textureName);

        void useTextureForObject(std::string objName, std::string textureName);

        void useTextureForObjectList(std::vector<std::string> objectList, std::string textureName);

        void useTextureForShape(std::string objName, std::string shapeName, std::string textureName);

        void usetextureForShapeList(std::string objName, std::vector<std::string> shapeList, std::string textureName);

        unsigned int getTextureID(std::string textureName);

        bool textureExist(std::string textureName);

        void deleteTexture(std::string textureName);

        
        // other stuff
        void listShapes(std::string objName);

        void setObjTransfrom(std::string objName, glm::mat4 modelMatrix);

        void cleanup();

        ~renderBase();

    protected:
        bool isInit = false; // so we don't initialize the renderer more than once

        std::vector<wavefrontObj> wavefrontObjList;

        std::vector<shader> shaders;
        const int MAX_SHADER_COUNT = 100;            
        std::vector<texture> textures;
        const int MAX_TEXTURE_COUNT = 100; 

        virtual void setupDefaultShader() = 0;
        virtual void setupEmptyTexture();

        void initGLBuffers(wavefrontObj &obj, bool calcNormals=false, bool flipNormals=false);

        void useShaderForShape(wavefrontObj &obj, std::string shapeName, std::string shaderName);
        void useShaderForObject(wavefrontObj &obj, std::string shaderName);

        void useTextureForShape(wavefrontObj &obj, std::string shapeName, std::string textureName);
        void useTextureForObject(wavefrontObj &obj, std::string textureName);

        void drawObj(wavefrontObj &obj);

        virtual void setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr) = 0;

        glm::vec3 getShapeCenter(wavefrontObj &obj, unsigned int shapeIdx);

};

} // namespace glr

#ifndef GLRENDER_STATIC   
    #include <glr/renderBase.cpp>
#endif

#endif