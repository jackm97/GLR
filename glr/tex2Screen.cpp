#include <glr/tex2Screen.h>

#include <string>

namespace glr {

GLRENDER_INLINE tex2Screen::tex2Screen()
{
    std::string vCode =
    #include <glr/shaders/tex2Screen.vs>
    ;
    std::string fCode =
    #include <glr/shaders/tex2Screen.fs>
    ;
    
    shaderList.push_back( shader(vCode.c_str(), fCode.c_str(), "default", RAW_CODE) );

    // vertices
    float vertices[] = {
    // positions            // texture coords
    1.0f,  1.0f, 0.0f,   1.0f, 1.0f,   // top right
    1.0f, -1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLRENDER_INLINE void tex2Screen::addShader(std::string vertPath, std::string fragPath, std::string shaderName)
{
    shaderList.push_back( shader (fragPath.c_str(),vertPath.c_str(),shaderName.c_str()) );
}

GLRENDER_INLINE void tex2Screen::useShader(std::string shaderName)
{
    for (int s=0; s < shaderList.size(); s++)
    {
        if (shaderList[s].name == shaderName){shaderIdx = s; break;}
    }
}

GLRENDER_INLINE void tex2Screen::addTexture(std::string texturePath, std::string textureName)
{
    texture newTexture(texturePath, textureName);
    textureList.push_back( newTexture );
    newTexture.glRelease();
}

GLRENDER_INLINE void tex2Screen::addTexture(int width, int height, std::string textureName)
{
    texture newTexture(width,height,textureName);
    textureList.push_back( newTexture );
    newTexture.glRelease();
}

GLRENDER_INLINE void tex2Screen::uploadPix2Tex(std::string textureName, GLenum format, GLenum type, void* data)
{
    for (int t=0; t < textureList.size(); t++)
    {
        if (textureList[t].name == textureName)
            textureList[t].loadPixels(format, type, data);
    }
}

GLRENDER_INLINE void tex2Screen::deleteTexture(std::string textureName)
{
    for (int t=0; t < textureList.size(); t++)
    {
        if (textureList[t].name == textureName){textureList.erase(textureList.begin() + t);}
    }
}

GLRENDER_INLINE void tex2Screen::draw()
{
    shaderList[shaderIdx].use();
    for (int t=0; t < textureList.size(); t++)
    {
        glActiveTexture(GL_TEXTURE0 + t);
        textureList[t].bind();
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    for (int t=0; t < textureList.size(); t++)
    {
        glActiveTexture(GL_TEXTURE0 + t);
        textureList[t].unbind();
    }
    glActiveTexture(GL_TEXTURE0);
}

} // namespace glr