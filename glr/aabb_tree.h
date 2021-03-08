#ifndef AABBTREE_H
#define AABBTREE_H
#include "glr_inline.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define TINYOBJ_CUSTOM_NAMESPACE glr
#include <glr/tinyobjloader/tiny_obj_loader.h>

#include <glr/shader.h>

#include <string>


namespace glr
{

// forward declaration
class OBJ;

struct AABBNode
{
    AABBNode *left_ = NULL, *right_ = NULL;

    glm::vec3 extent_{0.0f, 0.0f, 0.0f};

    glm::vec3 center_{0.0f, 0.0f, 0.0f};

    std::vector<tinyobj::index_t*> f_idx_list_; // face index pointers
};

class AABBTree
{
    public:
        AABBNode* head_ = NULL;

    public:
        AABBTree() {}

        AABBTree(OBJ* obj);

        void assignObj(OBJ* obj);
        
        void calcTree();

        void clearTree();

        void draw();

        void glRelease();

        ~AABBTree();

        friend void initialize();
        friend void cleanup();

        friend class OBJ;
        friend class sceneViewer;
    
    private:
        OBJ* obj_ptr_ = NULL;

        int num_aabb_;

        // static AABB shader
        static std::string aabb_vs_code_;
        static std::string aabb_fs_code_;
        static shader aabb_shader_;

        std::vector<unsigned int> vao_list_;
        std::vector<unsigned int> vbo_list_;
        bool is_loaded_into_gl_ = false;

    private:
        
        AABBNode* calcTree(std::vector<tinyobj::index_t*> f_idx_list);

        void clearTree(AABBNode* node);

        void initGLBuffers();

        void initGLBuffers(AABBNode* node);
};


} // namespace glr

#ifndef GLRENDER_STATIC
#   include <glr/aabb_tree.cpp>
#endif

#endif