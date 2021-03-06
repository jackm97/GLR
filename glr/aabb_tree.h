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

// forward declarations
class OBJ;
struct AABBNode;

class AABBTree
{
    public:
        AABBNode* head_ = NULL;

        // diagnostics
        int num_aabb_;
        int num_primitives_;
        float total_mem_;

        int N_v_; // number of volume overlap tests
        float C_v_; // average time cost of volume overlap test
        int num_leaf_overlap_ = 0; // number of leaf volumes that overlap

    public:
        AABBTree() {}

        AABBTree(OBJ* obj);

        void assignObj(OBJ* obj);
        
        void calcTree();

        void clearTree();

        bool intersectTest(AABBTree *other_tree);

        void draw();

        void glRelease();

        ~AABBTree();

        friend void initialize();
        friend void cleanup();

        friend class OBJ;
        friend class sceneViewer;
    
    private:
        OBJ* obj_ptr_ = NULL;

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

        bool intersectTest(AABBNode* A, glm::vec3 axis_A[3], AABBNode* B, glm::vec3 axis_B[3]);

        void clearIntersectTest();

        void initGLBuffers();

        void initGLBuffers(AABBNode* node);
};

struct AABBNode
{
    AABBTree* tree_ = NULL;

    AABBNode *left_ = NULL, *right_ = NULL;

    glm::vec3 extent_{0.0f, 0.0f, 0.0f};

    glm::vec3 center_{0.0f, 0.0f, 0.0f};

    bool is_intersect = false;

    float volume() {return (extent_.x * extent_.y * extent_.z * 2);}
};


} // namespace glr

#ifndef GLRENDER_STATIC
#   include <glr/aabb_tree.cpp>
#endif

#endif