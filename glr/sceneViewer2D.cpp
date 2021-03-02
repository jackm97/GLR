#include <glr/sceneViewer2D.h>

#include <string>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

namespace glr {

GLRENDER_INLINE sceneViewer2D::sceneViewer2D()
{

}

GLRENDER_INLINE void sceneViewer2D::init()
{
	
    if (!isInit){
		isInit = true;

		shaders.reserve(MAX_SHADER_COUNT);
		textures.reserve(MAX_TEXTURE_COUNT);

		setupEmptyTexture();

        setupDefaultShader();

        addTexture(1, 1, "background");
	}
    else
    {
        return;
    }

    // vertices
    float vertices[] = {
    // positions            // texture coords
    1.0f,  1.0f, 0.5f,   1.0f, 1.0f,   // top right
    1.0f, -1.0f, 0.5f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.5f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.5f,   0.0f, 1.0f    // top left 
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

GLRENDER_INLINE void sceneViewer2D::setBounds(float boundX, float boundY)
{
    this->boundX = boundX;
    this->boundY = boundY;
}

GLRENDER_INLINE void sceneViewer2D::drawScene()
{
    int shaderIdx;
    for (int s=0; s < shaders.size(); s++)
    {
        if (shaders[s].name == "default"){shaderIdx = s; break;}
    }
    shaders[shaderIdx].use();

	// model matrix
    glm::mat4 m(1.0f);
	int uLocation = glGetUniformLocation(shaders[shaderIdx].ID, "m");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(m));

    // set bounds
    shaders[shaderIdx].setInt("isBackground", 1);
    shaders[shaderIdx].setFloat("boundX", boundX);
    shaders[shaderIdx].setFloat("boundY", boundY);

    // matrial info
    float Kd[3]{1,1,1};
    shaders[shaderIdx].setVec3("Kd", Kd);
    shaders[shaderIdx].setInt("textureAssigned", 1);
    
    int textureIdx;
    for (int t=0; t < textures.size(); t++)
    {
        if (textures[t].name == "background"){textureIdx = t; break;}
    }
    textures[textureIdx].bind();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    textures[textureIdx].unbind();  
    
	for (int obj = 0; obj < wavefrontObjList.size(); obj++)
		drawObj(wavefrontObjList[obj]);  
}

GLRENDER_INLINE void sceneViewer2D::setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr)
{

	// model matrix
	int uLocation = glGetUniformLocation(shaderPtr->ID, "m");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(obj.modelMatrix));

    // set bounds
    shaderPtr->setInt("isBackground", 0);
    shaderPtr->setFloat("boundX", boundX);
    shaderPtr->setFloat("boundY", boundY);
	
	// material info
	shaderPtr->setVec3("Kd",  mat.diffuse);
}

GLRENDER_INLINE void sceneViewer2D::setupDefaultShader()
{
    // the shaders are included as .h
    // files containing the char array
    // of the shader so that the path
    // to the shaders are not required
    std::string vCode =
    #include <glr/shaders/sceneViewer2D.vs.h>
    ;
    std::string fCode =
    #include <glr/shaders/sceneViewer2D.fs.h>
    ;
    
    shaders.push_back( shader(vCode.c_str(), fCode.c_str(), "default", RAW_CODE) );
}

} // namespace glr