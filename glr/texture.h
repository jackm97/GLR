#ifndef TEXTURE_H
#define TEXTURE_H
#include "glr_inline.h"

#include <iostream>
#include <string>

typedef unsigned int GLenum;

namespace glr {
class texture{
    public:
        int ID_=-1;
        int width_, height_;
        std::string name_ = "";
        
        // creates 1x1 GL_RGB white texture object
        // with name=""
        // texture is unbound after creation
        texture();

        texture(const texture &src);
        
        // creates GL_RGB texture object from image path
        // with name
        // texture is unbound after creation
        texture(std::string img_path, std::string name);

        // creates 1x1 GL_RGB white texture object
        // with name
        // texture is unbound after creation
        texture(std::string name);

        // creates mxn GL_RGB white texture object
        // texture is unbound after creation
        // with name
        texture(int m, int n, std::string name);

        // creates GL_RGB texture object from image path
        // texture is unbound after creation
        // old texture is deleted
        void genNewTexture(std::string img_path);

        // creates 1x1 GL_RGB white texture object
        // texture is unbound after creation
        // old texture is deleted
        void genNewTexture();

        // creates mxn GL_RGB white texture object
        // texture is unbound after creation
        // old texture is deleted
        void genNewTexture(int m, int n);

        // loads pixel data with given format
        //
        // must match size of texture created
        // on object creation
        // 
        // the GL internal format of the texture
        // is still GL_RGB
        //
        // texture is left in the bound state after
        void loadPixels(GLenum format, GLenum type, void* data);

        void bind() const;

        void unbind() const;

        void operator=(const texture &src);

        // release texture object from gpu mem
        void glRelease();

        ~texture();

    private:
        void initializeEmpty(int m, int n);
};
}

#ifndef GLRENDER_STATIC
    #include <glr/texture.cpp>
#endif

#endif