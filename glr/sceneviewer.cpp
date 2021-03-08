#include <glr/sceneviewer.h>

#include<glr/aabb_tree.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

namespace glr {

GLRENDER_INLINE void sceneViewer::init()
{
	if (!is_init_){
		is_init_ = true;

		shaders_.reserve(MAX_SHADER_COUNT);
		textures_.reserve(MAX_TEXTURE_COUNT);

		setupEmptyTexture();

		setupDefaultShader();
	}
}

/*
*
*
CAMERA STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::useOrtho(float left, float right, float bottom, float top, float z_near, float z_far)
{
	proj_ = glm::ortho(left, right, bottom, top, z_near, z_far);
}

GLRENDER_INLINE void sceneViewer::usePerspective(float fovy, float aspect, float z_near, float z_far)
{
	proj_ = glm::perspective(fovy, aspect, z_near, z_far);
}

GLRENDER_INLINE void sceneViewer::addCamera(std::string name, glm::vec3 eye_pos, glm::vec3 look_dir, glm::vec3 up_dir)
{
	camera* cam_ptr = getCamera(name);

	camera new_cam;

	new_cam.name = name;
	new_cam.pos = eye_pos;
	new_cam.dir = glm::normalize(look_dir);
	new_cam.up = glm::normalize(up_dir);

	if (cam_ptr == NULL)
		camera_list_.push_back(new_cam);
	else
		*cam_ptr = new_cam;
}

GLRENDER_INLINE camera* sceneViewer::getCamera(std::string name)
{
	camera* cam_ptr = NULL;
	for (int i=0; i < camera_list_.size(); i++)
	{
		if (name == camera_list_[i].name)
		{
			cam_ptr = &(camera_list_[i]);
			break;
		}
	}

	return cam_ptr;
}

GLRENDER_INLINE camera* sceneViewer::getActiveCamera()
{
	return &(camera_list_[camera_idx_]);
}

GLRENDER_INLINE std::vector<camera*> sceneViewer::cameraList()
{
	std::vector<camera*> cam_list;

	for (int i = 0; i < camera_list_.size(); i++)
		cam_list.push_back(&(camera_list_[i]));

	return cam_list;
}

GLRENDER_INLINE void sceneViewer::useCamera(std::string name)
{
	for (int i=0; i < camera_list_.size(); i++)
	{
		if (name == camera_list_[i].name)
		{
			camera_idx_ = i;
			return;
		}
	}
}

/*
*
*
LIGHTING STUFF
*
*
*/
GLRENDER_INLINE void sceneViewer::setAmbientLight(glm::vec3 color, float I)
{
	ambient_color_ = color;
	ambient_I_ = I;
}
GLRENDER_INLINE void sceneViewer::addDirLight(glm::vec3 dir, glm::vec3 color, float I, float spec)
{
	dirLight new_light;

	new_light.dir = dir;
	new_light.color = color;
	new_light.I = I;
	new_light.spec = spec;

	dir_light_list_.push_back(new_light);
}

GLRENDER_INLINE void sceneViewer::addPointLight(glm::vec3 pos, glm::vec3 color, float I, float spec)
{
	pointLight new_light;

	new_light.pos = pos;
	new_light.color = color;
	new_light.I = I;
	new_light.spec = spec;

	point_light_list_.push_back(new_light);
}

GLRENDER_INLINE void sceneViewer::addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float I, float cutoff_angle, float spec)
{
	spotLight new_light;

	new_light.pos = pos;
	new_light.dir = dir;
	new_light.color = color;
	new_light.I = I;
	new_light.spec = spec;
	new_light.cutoff_angle = cutoff_angle;

	spotlight_list_.push_back(new_light);
}

/*
*
*
OTHER STUFF
*
*
*/

GLRENDER_INLINE glm::mat4 sceneViewer::modelMatrix()
{
	return model_;
}

GLRENDER_INLINE void sceneViewer::modelMatrix(const glm::mat4 m)
{
	model_ = m;
}

GLRENDER_INLINE void sceneViewer::drawScene()
{
	std::vector<glm::mat4> tmp;
	for (int obj = 0; obj < obj_list_.size(); obj++)
	{
		tmp.push_back(obj_list_[obj]->model_matrix_);
		obj_list_[obj]->model_matrix_ = model_ * obj_list_[obj]->model_matrix_;
	}

	setUniforms();

	for (int obj = 0; obj < obj_list_.size(); obj++)
		obj_list_[obj]->draw();

	for (int obj = 0; obj < obj_list_.size(); obj++)
	{
		obj_list_[obj]->model_matrix_ = tmp[obj];
	}
}

GLRENDER_INLINE void sceneViewer::setUniforms()
{
	for (int i = 0; i < obj_list_.size(); i++)
	{
		for (int j = 0; j < obj_list_[i]->shader_list_.size(); j++)
		{
			shader* shader_ptr = obj_list_[i]->shader_list_[j];
			shader_ptr->use();

			// camera stuff
			camera* cam_ptr = getActiveCamera();
			view_ = glm::lookAt(cam_ptr->pos, cam_ptr->pos + cam_ptr->dir, cam_ptr->up);
			unsigned int uLocation = glGetUniformLocation(shader_ptr->ID_, "cameraPos");
			glUniform3fv(uLocation, 1, glm::value_ptr(cam_ptr->pos));

			// view projection  matrix
			uLocation = glGetUniformLocation(shader_ptr->ID_, "v");
			glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(view_));
			uLocation = glGetUniformLocation(shader_ptr->ID_, "p");
			glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(proj_));

			// ambient light
			uLocation = glGetUniformLocation(shader_ptr->ID_, "ambientLightColor");
			glUniform3fv(uLocation, 1, glm::value_ptr(ambient_color_));
			uLocation = glGetUniformLocation(shader_ptr->ID_, "ambientI");
			glUniform1f(uLocation, ambient_I_);


			// directional lights
			std::vector<float> dirs;
			std::vector<float> colors;
			std::vector<float> Is;
			std::vector<float> specs;
			for (int l=0; l < dir_light_list_.size(); l++)
			{
				dirs.push_back(dir_light_list_[l].dir.x);
				dirs.push_back(dir_light_list_[l].dir.y);
				dirs.push_back(dir_light_list_[l].dir.z);
				
				colors.push_back(dir_light_list_[l].color.x);
				colors.push_back(dir_light_list_[l].color.y);
				colors.push_back(dir_light_list_[l].color.z);

				Is.push_back(dir_light_list_[l].I);
				specs.push_back(dir_light_list_[l].spec);
			}
			uLocation = glGetUniformLocation(shader_ptr->ID_, "dirLightDir");
			glUniform3fv(uLocation, dir_light_list_.size(), dirs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "dirLightColor");
			glUniform3fv(uLocation, dir_light_list_.size(), colors.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "dirLightI");
			glUniform1fv(uLocation, dir_light_list_.size(), Is.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "dirLightSpec");
			glUniform1fv(uLocation, dir_light_list_.size(), specs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "numDirLight");
			glUniform1i(uLocation, dir_light_list_.size());

			// point lights
			std::vector<float> pos;
			colors.clear();
			Is.clear();
			specs.clear();
			for (int l=0; l < point_light_list_.size(); l++)
			{
				pos.push_back(point_light_list_[l].pos.x);
				pos.push_back(point_light_list_[l].pos.y);
				pos.push_back(point_light_list_[l].pos.z);
				
				colors.push_back(point_light_list_[l].color.x);
				colors.push_back(point_light_list_[l].color.y);
				colors.push_back(point_light_list_[l].color.z);

				Is.push_back(point_light_list_[l].I);
				specs.push_back(point_light_list_[l].spec);
			}
			uLocation = glGetUniformLocation(shader_ptr->ID_, "pointLightPos");
			glUniform3fv(uLocation, point_light_list_.size(), pos.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "pointLightColor");
			glUniform3fv(uLocation, point_light_list_.size(), colors.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "pointLightI");
			glUniform1fv(uLocation, point_light_list_.size(), Is.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "pointLightSpec");
			glUniform1fv(uLocation, point_light_list_.size(), specs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "numPointLight");
			glUniform1i(uLocation, point_light_list_.size());

			// spot lights
			pos.clear();
			dirs.clear();
			colors.clear();
			Is.clear();
			specs.clear();
			std::vector<float> cutoffs;
			for (int l=0; l < spotlight_list_.size(); l++)
			{
				pos.push_back(spotlight_list_[l].pos.x);
				pos.push_back(spotlight_list_[l].pos.y);
				pos.push_back(spotlight_list_[l].pos.z);

				dirs.push_back(spotlight_list_[l].dir.x);
				dirs.push_back(spotlight_list_[l].dir.y);
				dirs.push_back(spotlight_list_[l].dir.z);
				
				colors.push_back(spotlight_list_[l].color.x);
				colors.push_back(spotlight_list_[l].color.y);
				colors.push_back(spotlight_list_[l].color.z);

				Is.push_back(spotlight_list_[l].I);
				specs.push_back(spotlight_list_[l].spec);

				cutoffs.push_back(spotlight_list_[l].cutoff_angle);
			}
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightPos");
			glUniform3fv(uLocation, spotlight_list_.size(), pos.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightDir");
			glUniform3fv(uLocation, spotlight_list_.size(), dirs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightColor");
			glUniform3fv(uLocation, spotlight_list_.size(), colors.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightI");
			glUniform1fv(uLocation, spotlight_list_.size(), Is.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightSpec");
			glUniform1fv(uLocation, spotlight_list_.size(), specs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "spotLightCutoff");
			glUniform1fv(uLocation, spotlight_list_.size(), cutoffs.data());
			uLocation = glGetUniformLocation(shader_ptr->ID_, "numSpotLight");
			glUniform1i(uLocation, spotlight_list_.size());
		}
	}

	

	// view projection  matrix
	shader* shader_ptr = &AABBTree::aabb_shader_;
	shader_ptr->use();
	int uLocation = glGetUniformLocation(shader_ptr->ID_, "v");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(view_));
	uLocation = glGetUniformLocation(shader_ptr->ID_, "p");
	glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(proj_));
}

GLRENDER_INLINE void sceneViewer::setupDefaultShader()
{
    // the shaders are included as .h
    // files containing the char array
    // of the shader so that the path
    // to the shaders are not required
    std::string v_code =
    #include <glr/shaders/sceneviewer.vs.h>
    ;
    std::string f_code =
    #include <glr/shaders/sceneviewer.fs.h>
    ;
    
	shader* shader_ptr = new shader(v_code.c_str(), f_code.c_str(), "default", RAW_CODE);
    shaders_.push_back( shader_ptr );
}

} // namespace glr