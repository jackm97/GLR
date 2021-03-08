#include <glr/texture.h>

#define CUSTOM_STB_IMAGE_IMPLEMENTATION
#define CUSTOM_STB_IMAGE_STATIC
#define STB_CUSTOM_NAMESPACE glr
#include <glr/stb/stb_image.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

namespace glr {

GLRENDER_INLINE texture::texture()
{
    this->name_ = "";
    initializeEmpty(1,1);
}

GLRENDER_INLINE texture::texture(const texture &src)
{
    *this = src;
}

GLRENDER_INLINE texture::texture(std::string img_path, std::string name)
{
    this->name_ = name;
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int n_channels;
    unsigned char *data = stbi_load(img_path.c_str(), &width_, &height_, &n_channels, 3);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ID_ = texture;
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLRENDER_INLINE texture::texture(std::string name)
{
    this->name_ = name;

    initializeEmpty(1,1);
}

GLRENDER_INLINE  texture::texture(int m, int n, std::string name)
{
    this->name_ = name;

    initializeEmpty(m,n);
}

GLRENDER_INLINE void texture::genNewTexture(std::string img_path)
{
    glDeleteTextures(1, (GLuint*) &ID_);
    texture new_texture(img_path, this->name_);

    *this = new_texture;
}

GLRENDER_INLINE void texture::genNewTexture()
{
    glDeleteTextures(1, (GLuint*) &ID_);
    texture new_texture(this->name_);

    *this = new_texture;
}

GLRENDER_INLINE void texture::genNewTexture(int m, int n)
{
    glDeleteTextures(1, (GLuint*) &ID_);
    texture new_texture(m, n, this->name_);

    *this = new_texture;
}

GLRENDER_INLINE void texture::loadPixels(GLenum format, GLenum type, void* data)
{
    this->bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, format, type, data);
    this->unbind();
}

GLRENDER_INLINE void texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, ID_);
}

GLRENDER_INLINE void texture::unbind() const

{
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLRENDER_INLINE void texture::operator=(const texture &src)
{
    name_ = src.name_;
    unsigned char* data = (unsigned char*) malloc(3 * src.height_ * src.width_ * sizeof(unsigned char));
    src.bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    src.unbind();
    glRelease();

    initializeEmpty(src.width_, src.height_);
    loadPixels(GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data);
}

GLRENDER_INLINE texture::~texture()
{
    glRelease();
}

// PRIVATE:

GLRENDER_INLINE void texture::initializeEmpty(int m, int n)
{    
    width_ = m;
    height_ = n;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char* data = new unsigned char[m*n*3];
    for (int i=0; i < m*n; i++)
    {
        data[3*i + 0] = 255;
        data[3*i + 1] = 255;
        data[3*i + 2] = 255;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete [] data;

    ID_ = texture;
}

GLRENDER_INLINE void texture::glRelease()
{
    if (ID_!=-1)
        glDeleteTextures(1, (GLuint *) &ID_);
    ID_ = -1;
}
}