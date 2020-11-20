#include <glr/sceneViewer2D.h>

#include <string>

namespace glr {

GLRENDER_INLINE sceneViewer2D::sceneViewer2D()
{

}

GLRENDER_INLINE void sceneViewer2D::init(float boundX, float boundY)
{
    this->boundX = boundX;
    this->boundY = boundY;
	
    if (!isInit){
		isInit = true;

		shaders.reserve(MAX_SHADER_COUNT);
		textures.reserve(MAX_TEXTURE_COUNT);

		// assign an empty texture for shapes without textures
		// used within the draw function to determine if
		// a texture should be used in the fragment shader
		if (!textureExist("empty"))
		{	
			texture emptyTexture("empty");
			textures.push_back(emptyTexture);
			emptyTexture.glRelease();
		}

        std::string vCode =
        #include <glr/shaders/sceneViewer2D.vs.h>
        ;
        std::string fCode =
        #include <glr/shaders/sceneViewer2D.fs.h>
        ;
        
        shaders.push_back( shader(vCode.c_str(), fCode.c_str(), "default", RAW_CODE) );
	}
    else
    {
        glDeleteBuffers(1, (GLuint*) &EBO);
        glDeleteBuffers(1, (GLuint*) &VBO);
        glDeleteVertexArrays(1, (GLuint*) &VAO);
    }

    // vertices
    float vertices[] = {
    // positions            // texture coords
    1.0f/boundX,  1.0f/boundY, 0.5f,   1.0f, 1.0f,   // top right
    1.0f/boundX, -1.0f/boundY, 0.5f,   1.0f, 0.0f,   // bottom right
    -1.0f/boundX, -1.0f/boundY, 0.5f,   0.0f, 0.0f,   // bottom left
    -1.0f/boundX,  1.0f/boundY, 0.5f,   0.0f, 1.0f    // top left 
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

GLRENDER_INLINE void sceneViewer2D::drawScene()
{
    int shaderIdx;
    for (int s=0; s < shaders.size(); s++)
    {
        if (shaders[s].name == "default"){shaderIdx = s; break;}
    }
    shaders[shaderIdx].use();

    shaders[shaderIdx].setFloat("boundX", boundX);
    shaders[shaderIdx].setFloat("boundY", boundY);
    
    int textureIdx;
    for (int t=0; t < textures.size(); t++)
    {
        if (textures[t].name == "background"){textureIdx = t; break;}
    }
    textures[textureIdx].bind();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    textures[textureIdx].unbind();    
}

} // namespace glr