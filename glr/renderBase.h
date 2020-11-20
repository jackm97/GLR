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

        void deleteTexture(std::string textureName);

        
        // other stuff
        void listShapes(std::string objName);

        void setObjTransfrom(std::string objName, glm::mat4 modelMatrix);

        ~renderBase(){}

    protected:
        bool isInit = false; // so we don't initialize the renderer more than once

        std::vector<wavefrontObj> wavefrontObjList;

        std::vector<shader> shaders;
        const int MAX_SHADER_COUNT = 100;            
        std::vector<texture> textures;
        const int MAX_TEXTURE_COUNT = 100; 

        void initGLBuffers(wavefrontObj &obj, bool calcNormals=false, bool flipNormals=false);

        void useShaderForShape(wavefrontObj &obj, std::string shapeName, std::string shaderName);
        void useShaderForObject(wavefrontObj &obj, std::string shaderName);

        void usetextureForShape(wavefrontObj &obj, std::string shapeName, std::string textureName);
        void usetextureForObject(wavefrontObj &obj, std::string textureName);

        void drawObj(wavefrontObj &obj);

        virtual void setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr){};

        glm::vec3 getShapeCenter(wavefrontObj &obj, unsigned int shapeIdx);

};

#ifndef GLRENDER_STATIC   
    #include <glr/renderBase.cpp>
#endif

} // namespace glr

#endif