#include <glr/texture.h>

#define CUSTOM_STB_IMAGE_IMPLEMENTATION
#define CUSTOM_STB_IMAGE_STATIC
#define STB_CUSTOM_NAMESPACE glr
#include <glr/stb/stb_image.h>

namespace glr {

GLRENDER_INLINE texture::texture()
{
    this->name = "";
    initializeEmpty(1,1);
}

GLRENDER_INLINE texture::texture(const texture &src)
{
    *this = src;
}

GLRENDER_INLINE texture::texture(std::string imagePath, std::string name)
{
    this-> name = name;
    
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int nrChannels;
    unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 3);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ID = texture;
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLRENDER_INLINE texture::texture(std::string name)
{
    this-> name = name;

    initializeEmpty(1,1);
}

GLRENDER_INLINE  texture::texture(int m, int n, std::string name)
{
    this->name = name;

    initializeEmpty(m,n);
}

GLRENDER_INLINE void texture::genNewTexture(std::string imagePath)
{
    glDeleteTextures(1, (GLuint*) &ID);
    texture newTexture(imagePath, this->name);

    *this = newTexture;
}

GLRENDER_INLINE void texture::genNewTexture()
{
    glDeleteTextures(1, (GLuint*) &ID);
    texture newTexture(this->name);

    *this = newTexture;
}

GLRENDER_INLINE void texture::genNewTexture(int m, int n)
{
    glDeleteTextures(1, (GLuint*) &ID);
    texture newTexture(m, n, this->name);

    *this = newTexture;
}

GLRENDER_INLINE void texture::loadPixels(GLenum format, GLenum type, void* data)
{
    this->bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);
    this->unbind();
}

GLRENDER_INLINE void texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, ID);
}

GLRENDER_INLINE void texture::unbind() const

{
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLRENDER_INLINE void texture::operator=(const texture &src)
{
    name = src.name;
    unsigned char* data = (unsigned char*) malloc(3 * src.height * src.width * sizeof(unsigned char));
    src.bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    src.unbind();
    glRelease();

    initializeEmpty(src.width, src.height);
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
    width = m;
    height = n;

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    delete [] data;

    ID = texture;
}

GLRENDER_INLINE void texture::glRelease()
{
    if (ID!=-1)
        glDeleteTextures(1, (GLuint *) &ID);
    ID = -1;
}
}