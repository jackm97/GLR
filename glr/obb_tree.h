#ifndef OBBTREE_H
#define OBBTREE_H
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
struct OBBNode;

class OBBTree
{
    public:
        OBBNode* head_ = NULL;

        // diagnostics
        int num_obb_ = 0;
        int num_primitives_ = 0;
        float total_mem_ = 0;

        int N_v_ = 0; // number of volume overlap tests
        float C_v_ = 0; // average time cost of volume overlap test
        int num_leaf_overlap_ = 0; // number of leaf volumes that overlap

    public:
        OBBTree() {}

        OBBTree(OBJ* obj);

        void assignObj(OBJ* obj);
        
        void calcTree();

        void clearTree();

        bool intersectTest(OBBTree *other_tree);

        void draw();

        void glRelease();

        ~OBBTree();

        friend void initialize();
        friend void cleanup();

        friend class OBJ;
        friend class sceneViewer;
    
    private:
        OBJ* obj_ptr_ = NULL;

        // static AABB shader
        static std::string obb_vs_code_;
        static std::string obb_fs_code_;
        static shader obb_shader_;

        std::vector<unsigned int> vao_list_;
        std::vector<unsigned int> vbo_list_;
        bool is_loaded_into_gl_ = false;

    private:
        
        OBBNode* calcTree(std::vector<tinyobj::index_t*> f_idx_list);

        glm::vec3 calcMean(const std::vector<tinyobj::index_t*>& f_idx_list);

        void calcOBBAxes(const std::vector<tinyobj::index_t*>& f_idx_list, glm::vec3 axes[3]);

        void clearTree(OBBNode* node);

        bool intersectTest(OBBNode* A, glm::vec3 axis_A[3], OBBNode* B, glm::vec3 axis_B[3]);

        void clearIntersectTest();

        void initGLBuffers();

        void initGLBuffers(OBBNode* node);
};

struct OBBNode
{
    OBBTree* tree_ = NULL;

    OBBNode *left_ = NULL, *right_ = NULL;

    glm::vec3 extent_{0.0f, 0.0f, 0.0f};

    glm::vec3 center_{0.0f, 0.0f, 0.0f};

    glm::vec3 axes_[3];

    bool is_intersect = false;

    float volume() {return (extent_.x * extent_.y * extent_.z * 2);}
};


} // namespace glr

#ifndef GLRENDER_STATIC
#   include <glr/obb_tree.cpp>
#endif

#endif