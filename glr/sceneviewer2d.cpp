#include <glr/sceneviewer2d.h>

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
	
    if (!is_init_){
		is_init_ = true;

		shaders_.reserve(MAX_SHADER_COUNT);
		textures_.reserve(MAX_TEXTURE_COUNT);

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
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLRENDER_INLINE void sceneViewer2D::setBounds(float bound_x, float bound_y)
{
    this->bound_x_ = bound_x;
    this->bound_y_ = bound_y;
}

GLRENDER_INLINE void sceneViewer2D::drawScene()
{
    shader* shader_ptr = getShader("default");

	// model matrix
    glm::mat4 m(1.0f);
	int uLocation = glGetUniformLocation(shader_ptr->ID_, "m");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(m));

    // set bounds
    shader_ptr->setInt("isBackground", 1);
    shader_ptr->setFloat("boundX", bound_x_);
    shader_ptr->setFloat("boundY", bound_y_);

    // matrial info
    float Kd[3]{1,1,1};
    shader_ptr->setVec3("Kd", Kd);
    shader_ptr->setInt("textureAssigned", 1);
    
    getTexture("background")->bind();

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    getTexture("background")->unbind();  

    setUniforms();
    
	for (int obj = 0; obj < obj_list_.size(); obj++)
		obj_list_[obj]->draw();  
}

GLRENDER_INLINE void sceneViewer2D::setUniforms()
{
	for (int i = 0; i < obj_list_.size(); i++)
	{
		for (int j = 0; j < obj_list_[i]->shader_list_.size(); j++)
		{
			shader* shader_ptr = obj_list_[i]->shader_list_[j];
			shader_ptr->use();

            // set bounds
            shader_ptr->setInt("isBackground", 0);
            shader_ptr->setFloat("boundX", bound_x_);
            shader_ptr->setFloat("boundY", bound_y_);
        }
    }
}

GLRENDER_INLINE void sceneViewer2D::setupDefaultShader()
{
    // the shaders are included as .h
    // files containing the char array
    // of the shader so that the path
    // to the shaders are not required
    std::string v_code =
    #include <glr/shaders/sceneviewer2d.vs.h>
    ;
    std::string f_code =
    #include <glr/shaders/sceneviewer2d.fs.h>
    ;
    
    shader* shader_ptr = new shader(v_code.c_str(), f_code.c_str(), "default", RAW_CODE);
    shaders_.push_back( shader_ptr );
}

} // namespace glr