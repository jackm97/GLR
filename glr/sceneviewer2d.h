#ifndef TEX2SCREEN_H
#define TEX2SCREEN_H
#include "glr_inline.h"

#include <glr/renderbase.h>

#include <glr/shader.h>
#include <glr/texture.h>

#include <vector>
#include <string>

namespace glr {

class sceneViewer2D : public renderBase {
    public:
        sceneViewer2D();

        void init();

        void setBounds(float bound_x, float bound_y);

        void drawScene();

    private:
        float bound_x_, bound_y_; // the viewport goes from x=0->bound_x_ and y=0->bound_y_

        unsigned int vbo_, vao_, ebo_;

        void setUniforms();

        void setupDefaultShader();
};

} //namespace glr

#ifndef GLRENDER_STATIC   
    #include <glr/sceneviewer2d.cpp>
#endif

#endif