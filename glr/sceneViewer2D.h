#ifndef TEX2SCREEN_H
#define TEX2SCREEN_H
#include "glr_inline.h"

#include <glr/shader.h>
#include <glr/texture.h>

#include <vector>
#include <string>

namespace glr {

class sceneViewer2D {
    public:
        sceneViewer2D();
        void init();

        void addShader(std::string vertPath, std::string fragPath, std::string shaderName);
        void useShader(std::string shaderName);

        void addTexture(std::string texturePath, std::string textureName);
        void addTexture(int width, int height, std::string textureName);
        void uploadPix2Tex(std::string textureName, GLenum format, GLenum type, void* data);
        void deleteTexture(std::string textureName);

        void draw();

    private:
        unsigned int shaderIdx = 0;
        std::vector<shader> shaderList;
        std::vector<texture> textureList;

        unsigned int VBO, VAO, EBO;
};

} //namespace glr

#ifndef GLRENDER_STATIC   
    #include <glr/sceneViewer2D.cpp>
#endif

#endif