#ifndef OBJ_H
#define OBJ_H

#include "glr_inline.h"

#include <glr/texture.h>
#include <glr/shader.h>
#include <glr/aabb_tree.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <string>


#define TINYOBJ_CUSTOM_NAMESPACE glr
#include <glr/tinyobjloader/tiny_obj_loader.h>

namespace glr {

// forward declarations for friend classes
class renderBase;
class sceneViewer;
class sceneViewer2D;

class OBJ
{
    public:
        std::string obj_path_ = "", base_dir_ = "";

        std::string name_ = "";
        
        tinyobj::attrib_t attrib_; // see tinyobj docs
        std::vector<tinyobj::shape_t> shapes_; // see tinyobj docs
        std::vector<tinyobj::material_t> materials_; // see tinyobj docs

        std::vector<glm::vec3> shape_centers_;
        std::vector<float> shape_radii_; // radii for unscaled shapes

        glm::vec3 center_; // center of entire obj
        float radius_; // radius of unscaled obj

        friend renderBase;
        friend sceneViewer;
        friend sceneViewer2D;

    public:
        OBJ() {}

        // copy and assignment operator overload
        OBJ(const OBJ &src);

        void operator=(const OBJ &src);

        OBJ(std::string obj_path, std::string base_dir, std::string obj_name="", bool calc_normals=false, bool flip_normals=false);

        void loadFromObj(std::string obj_path, std::string base_dir, std::string obj_name="", bool calc_normals=false, bool flip_normals=false);

        void initGLBuffers(bool calc_normals, bool flip_normals);
        
        // vertex colors
        void setVertColor(float color[3]);

        void setVertColorForShape(std::string shape_name, float color[3]);

        void useVertColor(bool use);

        void useVertColorForShape(std::string shape_name, bool use);
        
        // shaders
        void setShader(shader* shader_ptr);

        void setShaderForShape(std::string shape_name, shader* shader_ptr);
        
        // texture
        void setTexture(texture* texture_ptr);

        void setTextureForShape(std::string shape_name, texture* texture_ptr);

        // object info
        glm::mat4 modelMatrix();

        void modelMatrix(glm::mat4 mat);

        // geometry
        void calcAABB();

        void displayAABB(bool use);

        // draw object
        void draw();

        // destructor and OpenGL release mem
        void glRelease();

        ~OBJ();

    private:

        glm::mat4 model_matrix_{1.0f};

        std::vector<bool> no_uv_map_;

        // This vector has same size as shapes
        // and each entry is a pointer to a 
        // glr::shader object
        std::vector<shader*> shader_list_;

        // works similar to shaderPtrs
        std::vector<texture*> texture_list_;
        std::vector<bool> textures_assigned_;

        std::vector<unsigned int> vao_list_;
        std::vector<unsigned int> vbo_list_;
        bool is_loaded_into_gl_ = false;

        // same size as shapes
        std::vector<bool> use_vert_colors_;

        bool is_calc_normals_ = false;
        bool is_flip_normals_ = false;

        AABBTree aabb_tree_;
        bool aabb_tree_enabled_ = false;
        bool display_aabb_tree_ = false;

    private:

        void setUniforms(unsigned int shapde_idx, tinyobj::material_t &mat, shader* shader_ptr);

        void calcCenters();
};

} // namespace glr

#ifndef GLRENDER_STATIC
#   include <glr/OBJ.cpp>
#endif

#endif