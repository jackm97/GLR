#ifndef TEX2SCREEN_H
#define TEX2SCREEN_H
#include "glr_inline.h"

#include <glr/renderBase.h>

#include <glr/shader.h>
#include <glr/texture.h>

#include <vector>
#include <string>

namespace glr {

class sceneViewer2D : public renderBase {
    public:
        sceneViewer2D();

        void init(float boundX, float boundY);

        void changeBounds(float boundX, float boundY);

        void drawScene();

    private:
        float boundX, boundY; // the viewport goes from x=0->boundX and y=0->boundY

        unsigned int VBO, VAO, EBO;

        void setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr);
};

} //namespace glr

#ifndef GLRENDER_STATIC   
    #include <glr/sceneViewer2D.cpp>
#endif

#endif