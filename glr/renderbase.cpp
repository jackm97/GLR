#include <glr/renderbase.h>

#define CUSTOM_TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_INLINE GLRENDER_INLINE
#include <glr/tinyobjloader/tiny_obj_loader.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <cstring>
#include <string>
#include <iostream>
#include <vector>

namespace glr {

GLRENDER_INLINE void renderBase::addOBJ(std::string obj_path, std::string base_dir, std::string obj_name, bool calc_normals, bool flip_normals)
{
	OBJ* new_obj = NULL;

	if (OBJExist(obj_name))
	{
		new_obj = getOBJ(obj_name);
		*new_obj = OBJ(obj_path, base_dir, obj_name, calc_normals, flip_normals);
	}
	else
	{
		new_obj = new OBJ(obj_path, base_dir, obj_name, calc_normals, flip_normals);

		obj_list_.push_back(new_obj);
	}
	
	for (int s = 0; s < new_obj->shapes_.size(); s++)
		for (size_t f = 0; f < new_obj->shapes_[s].mesh.num_face_vertices.size(); f++) {
			if (new_obj->shapes_[s].mesh.num_face_vertices[f] != 3)
			{
				continue;
			}
			
			// add texture if there is one (repeated textures are not added again)
			std::string diffuse_texname = new_obj->materials_[new_obj->shapes_[s].mesh.material_ids[0]].diffuse_texname;
			if (diffuse_texname.length() != 0)
			{
				addTexture(base_dir + diffuse_texname, diffuse_texname);
				new_obj->setTextureForShape(new_obj->shapes_[s].name, getTexture(diffuse_texname));
			}
		}
	
	new_obj->setShader(getShader("default"));
}

GLRENDER_INLINE OBJ* renderBase::getOBJ(std::string obj_name)
{
	int obj;
	for (obj = 0; obj < obj_list_.size(); obj++)
		if (obj_list_[obj]->name_ == obj_name) return (obj_list_[obj]);

	return NULL;
}

GLRENDER_INLINE std::vector<OBJ*> renderBase::OBJList()
{
	return obj_list_;
}

GLRENDER_INLINE void renderBase::deleteOBJ(std::string obj_name)
{
	int obj;
	for (obj = 0; obj < obj_list_.size(); obj++)
		if (obj_list_[obj]->name_ == obj_name) break;

	delete obj_list_[obj];
	obj_list_.erase(obj_list_.begin() + obj);
}

GLRENDER_INLINE bool renderBase::OBJExist(std::string name)
{
	int obj;
	for (obj = 0; obj < obj_list_.size(); obj++)
		if (obj_list_[obj]->name_ == name) return true;

	return false;
}


/*
*
*
PUBLIC SHADER STUFF
*
*
*/
GLRENDER_INLINE void renderBase::addShader(std::string vert_shader_info, std::string frag_shader_info, std::string shader_name, shaderLoadType load_type)
{
	if (shaderExist(shader_name))
	{
		shader* shader_ptr = getShader(shader_name);
		*shader_ptr = shader(vert_shader_info.c_str(), frag_shader_info.c_str(), shader_name.c_str());
		return;
	}
	else
		if (shaders_.size() > MAX_SHADER_COUNT) std::cerr << "Exceded Maximum Shader Count in renderBase" << std::endl;

	shader* shader_ptr = new shader(vert_shader_info.c_str(), frag_shader_info.c_str(), shader_name.c_str());
    shaders_.push_back( shader_ptr );
}

GLRENDER_INLINE shader* renderBase::getShader(std::string shader_name)
{	
	for (int i=0; i<shaders_.size(); i++)
		if (shader_name == shaders_[i]->name_) return shaders_[i];

	return NULL;
}

GLRENDER_INLINE std::vector<shader*> renderBase::shaderList()
{
	return this->shaders_;
}

GLRENDER_INLINE void renderBase::deleteShader(std::string shader_name)
{
	int s;
    for (s = 0; s < shaders_.size(); s++)
    {
        if (shaders_[s]->name_ == shader_name) break;
    }

	delete shaders_[s];
	shaders_.erase(shaders_.begin() + s);
}

GLRENDER_INLINE bool renderBase::shaderExist(std::string shader_name)
{
	for (int i=0; i<shaders_.size(); i++)
		if (shader_name == shaders_[i]->name_) return true;

	return false;
}

/*
*
*
PUBLIC TEXTURE STUFF
*
*
*/
GLRENDER_INLINE void renderBase::addTexture(std::string texture_path, std::string texture_name)
{
	if (textureExist(texture_name))
	{
		texture* texture_ptr = getTexture(texture_name);
		*texture_ptr = texture(texture_path, texture_name);
		return;
	}
	else
		if (textures_.size() > MAX_TEXTURE_COUNT) std::cerr << "Exceded Maximum Texture Count in renderBase" << std::endl;
	
	texture* texture_ptr = new texture( texture_path, texture_name );
	textures_.push_back( texture_ptr );
}

GLRENDER_INLINE void renderBase::addTexture(int width, int height, std::string texture_name)
{
	if (textureExist(texture_name))
	{
		texture* texture_ptr = getTexture(texture_name);
		*texture_ptr = texture(width, height, texture_name);
		return;
	}
	else
		if (textures_.size() > MAX_TEXTURE_COUNT) std::cerr << "Exceded Maximum Texture Count in renderBase" << std::endl;
		
	texture* texture_ptr = new texture(width, height, texture_name);
	textures_.push_back( texture_ptr );
}

GLRENDER_INLINE texture* renderBase::getTexture(std::string texture_name)
{
	for (int i=0; i<textures_.size(); i++)
		if (texture_name == textures_[i]->name_) return textures_[i];

	return NULL;
}

GLRENDER_INLINE std::vector<texture*> renderBase::textureList()
{
	return this->textures_;
}

GLRENDER_INLINE void renderBase::deleteTexture(std::string texture_name)
{
	int t;
    for (t = 0; t < textures_.size(); t++)
    {
        if (textures_[t]->name_ == texture_name) break;
    }
	
	delete textures_[t];
	textures_.erase(textures_.begin() + t);
}

GLRENDER_INLINE bool renderBase::textureExist(std::string texture_name)
{
	for (int i=0; i<textures_.size(); i++)
		if (texture_name == textures_[i]->name_) return true;

	return false;
}


GLRENDER_INLINE void renderBase::cleanup()
{
	for (int s = 0; s < shaders_.size(); s++)
		delete shaders_[s];
	shaders_.clear();
	for (int t = 0; t < textures_.size(); t++)
		delete textures_[t];
	textures_.clear();
	for (int o = 0; o < obj_list_.size(); o++)
		delete obj_list_[o];
	obj_list_.clear();

	is_init_ = false;
}


GLRENDER_INLINE renderBase::~renderBase()
{
	if (is_init_)
	{
		std::cerr << "glr::renderBase: Need to call glr::renderBase::cleanup() before OpenGL context deletion" << std::endl;
	}
}


GLRENDER_INLINE void renderBase::setupEmptyTexture()
{
	// assign an empty texture for shapes without textures
	// used within the draw function to determine if
	// a texture should be used in the fragment shader
	if (!textureExist("empty"))
	{	
		texture* empty = new texture("empty");
		textures_.push_back(empty);
	}
}

} //namespace glr