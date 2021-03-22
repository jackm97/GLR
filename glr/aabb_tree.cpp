#include <glr/aabb_tree.h>
#include <glr/obj.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <stack>
#include <iostream>
#include <chrono>

namespace glr
{

std::string AABBTree::aabb_vs_code_(
    #include <glr/shaders/aabb.vs.h>
);

std::string AABBTree::aabb_fs_code_(
    #include <glr/shaders/aabb.fs.h>
);

shader AABBTree::aabb_shader_;

GLRENDER_INLINE AABBTree::AABBTree(OBJ* obj)
{
    assignObj(obj);
}

GLRENDER_INLINE void AABBTree::assignObj(OBJ* obj)
{
    this->obj_ptr_ = obj;
}

GLRENDER_INLINE void AABBTree::calcTree()
{
    clearTree();

    std::vector<tinyobj::index_t*> f_idx_list;

    for (int s = 0; s < obj_ptr_->shapes_.size(); s++)
    {
        int index_offset = 0;
        for (int f = 0; f < obj_ptr_->shapes_[s].mesh.num_face_vertices.size(); f++)
        {
            if (obj_ptr_->shapes_[s].mesh.num_face_vertices[f] != 3)
            {
                index_offset += obj_ptr_->shapes_[s].mesh.num_face_vertices[f];
                continue;
            }

            for (int v = 0; v < 3; v++)
            {
                f_idx_list.push_back(&(obj_ptr_->shapes_[s].mesh.indices[index_offset + v]));
            }

            index_offset += 3;
        }
    }

    head_ = calcTree(f_idx_list);

    initGLBuffers();
}

GLRENDER_INLINE void AABBTree::clearTree()
{
    clearTree(head_);
    glRelease();
    head_ = NULL;
    num_aabb_ = 0;
    total_mem_ = 0;
    num_primitives_ = 0;
    N_v_ = 0;
    C_v_ = 0;
    num_leaf_overlap_ = 0;
}

GLRENDER_INLINE void AABBTree::draw()
{
    AABBTree::aabb_shader_.use();
    for (int a = 0; a < vao_list_.size(); a++)
    {
		// draw
		glBindVertexArray(vao_list_[a]);
        std::vector<int> starts;
        std::vector<int> num_verts;
        for (int n = 0; n < 6*num_aabb_; n++)
        {
            starts.push_back(4 * n);
            num_verts.push_back(4);
        }
		glMultiDrawArrays(GL_LINE_LOOP, starts.data(), num_verts.data(), 6*num_aabb_);
		glBindVertexArray(0);
    }
}

GLRENDER_INLINE void AABBTree::initGLBuffers()
{
    glRelease();

    vao_list_.clear();
    vbo_list_.clear();

    initGLBuffers(head_);

    is_loaded_into_gl_ = true;
}

GLRENDER_INLINE void AABBTree::initGLBuffers(AABBNode* node)
{
    if (node==NULL)
        return;

	std::vector<float> vertex_data;
    vertex_data.reserve(4*3*num_aabb_);
    unsigned int VAO;
    unsigned int VBO;

    glGenVertexArrays(1, &VAO);
    vao_list_.push_back(VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    vbo_list_.push_back(VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glm::vec3 v1, v2, v3, v4;

    std::stack<AABBNode*> node_stack;

    node_stack.push(node);

    // int num_AABB = 0;
    while (!node_stack.empty())
    {
        node = node_stack.top();
        node_stack.pop();
        // num_AABB += 1;
        // printf("\rAABBs added to GPU: %i", num_AABB);
        // fflush(stdout);
        glm::vec3 color;
        float scale = 1;
        if (node->is_intersect)
        {
            color = glm::vec3(0, 1, 0);
            scale = 1.01;
        }
        else
            color = glm::vec3(0, 0, 0);

        if (node->left_ != NULL)
            node_stack.push(node->left_);
        if (node->right_ != NULL)
            node_stack.push(node->right_);

        // top
        v1 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(node->extent_.x, 0, -node->extent_.z);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(node->extent_.x, 0, node->extent_.z);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, node->extent_.z);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, -node->extent_.z);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // bottom
        v1 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(node->extent_.x, 0, -node->extent_.z);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(node->extent_.x, 0, node->extent_.z);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, node->extent_.z);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, -node->extent_.z);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // front
        v1 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(node->extent_.x, -node->extent_.y, 0);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(node->extent_.x, node->extent_.y, 0);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(-node->extent_.x, node->extent_.y, 0);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(-node->extent_.x, -node->extent_.y, 0);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // back
        v1 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(node->extent_.x, -node->extent_.y, 0);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(node->extent_.x, node->extent_.y, 0);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(-node->extent_.x, node->extent_.y, 0);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(-node->extent_.x, -node->extent_.y, 0);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // left
        v1 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, node->extent_.z);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, node->extent_.z);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, -node->extent_.z);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, -node->extent_.z);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // right
        v1 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, node->extent_.z);
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, node->extent_.z);
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, -node->extent_.z);
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, -node->extent_.z);
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), vertex_data.data(), GL_DYNAMIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // printf("\n");
}

GLRENDER_INLINE void AABBTree::glRelease()
{
	if (!is_loaded_into_gl_) return;

	glDeleteBuffers(this->vbo_list_.size(), this->vbo_list_.data());
	glDeleteVertexArrays(this->vao_list_.size(), this->vao_list_.data());

	this->vao_list_.clear();
	this->vbo_list_.clear();

    is_loaded_into_gl_ = false;
}

GLRENDER_INLINE AABBTree::~AABBTree()
{
	glRelease();
    clearTree();
}

GLRENDER_INLINE AABBNode* AABBTree::calcTree(std::vector<tinyobj::index_t*> f_idx_list)
{
    if (f_idx_list.size() == 0)
        return NULL;

    AABBNode* head = new AABBNode;

    std::stack<AABBNode*> node_stack;
    std::stack<std::vector<tinyobj::index_t*>> f_idx_list_stack;

    node_stack.push(head);
    f_idx_list_stack.push(f_idx_list);

    num_aabb_ = 0;
    total_mem_ = 0;
    num_primitives_ = f_idx_list.size()/3;

    while (!node_stack.empty())
    {
        AABBNode* node = node_stack.top();
        node_stack.pop();
        num_aabb_ += 1;
        total_mem_ += sizeof(*node);

        node->tree_ = this;

        f_idx_list = f_idx_list_stack.top();
        f_idx_list_stack.pop();

        float minP[3];
        float maxP[3];

        glm::vec3 p;
        for (int i = 0; i < 3; i++)
        {
            p[i] = obj_ptr_->attrib_.vertices[3 * f_idx_list[0]->vertex_index + i];
        }
        // p = glm::vec3(obj_ptr_->modelMatrix() * glm::vec4(p, 1.0f));
        for (int i = 0; i < 3; i++)
        {
            minP[i] = p[i];
            maxP[i] = p[i];
        }

        int f_num = f_idx_list.size()/3;
        for (int f = 0; f < f_num; f++)
        {
            for (int v = 0; v < 3; v++)
            {
                for (int i = 0; i < 3; i++)
                {
                    p[i] = obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + i];
                }
                // p = glm::vec3(obj_ptr_->modelMatrix() * glm::vec4(p, 1.0f));
                for (int i = 0; i < 3; i++)
                {
                    float v_i = p[i];
                    if (v_i > maxP[i])
                        maxP[i] = v_i;
                    if (v_i < minP[i])
                        minP[i] = v_i;
                }
            }
        }

        glm::vec3 center{
            (maxP[0] + minP[0])/2,
            (maxP[1] + minP[1])/2,
            (maxP[2] + minP[2])/2
        };

        node->center_ = center;

        glm::vec3 extent{
            (maxP[0] - minP[0])/2,
            (maxP[1] - minP[1])/2,
            (maxP[2] - minP[2])/2
        };

        node->extent_ = extent;

        if (f_num == 1)
            continue;

        enum PlaneSep {
            XY,
            YZ,
            ZX
        };

        PlaneSep plane_sep;
        if (extent.x >= extent.y && extent.x >= extent.z)
            plane_sep = YZ;
        else if (extent.y >= extent.x && extent.y >= extent.z)
            plane_sep = ZX;
        else if (extent.z >= extent.x && extent.z >= extent.y)
            plane_sep = XY;

        float delta; // loacation of splitting plane
        std::vector<float> axis_vals;
        for (int f = 0; f < f_num; f++)
        {
            glm::vec3 centroid{
                0,
                0,
                0
            };

            for (int v = 0; v < 3; v++)
            {
                glm::vec3 tmp{
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 0],
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 1],
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 2]
                };

                // tmp = glm::vec3(obj_ptr_->modelMatrix() * glm::vec4(tmp, 1.0f));

                centroid += tmp;
            }

            centroid /= 3;

            switch (plane_sep)
            {
            case YZ:
                axis_vals.push_back(centroid.x);
                break;
            case ZX:
                axis_vals.push_back(centroid.y);
                break;
            case XY:
                axis_vals.push_back(centroid.z);
                break;
            }
        }

        std::sort(axis_vals.data(), axis_vals.data() + axis_vals.size());
        if (axis_vals.size()%2 == 1)
        {
            delta = axis_vals[axis_vals.size()/2];
        }
        else
        {
            delta = axis_vals[axis_vals.size()/2 - 1] + axis_vals[axis_vals.size()/2];
            delta /= 2;
        }
        bool split_evenly = false;
        if (axis_vals[0] == axis_vals[axis_vals.size() - 1])
            split_evenly = true;

        std::vector<tinyobj::index_t*> f_idx_list_l;
        std::vector<tinyobj::index_t*> f_idx_list_r;
        for (int f = 0; f < f_num; f++)
        {

            if (split_evenly)
            {
                for (int v = 0; v < 3; v++)
                    if (f < f_num/2)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                    else
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                continue;
            }
            
            glm::vec3 centroid{
                0,
                0,
                0
            };

            for (int v = 0; v < 3; v++)
            {
                glm::vec3 tmp{
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 0],
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 1],
                    obj_ptr_->attrib_.vertices[3 * f_idx_list[3*f + v]->vertex_index + 2]
                };

                // tmp = glm::vec3(obj_ptr_->modelMatrix() * glm::vec4(tmp, 1.0f));

                centroid += tmp;
            }

            centroid /= 3;

            switch (plane_sep)
            {
            case YZ:
                if ( (centroid.x >= delta && axis_vals[0] != delta) || centroid.x > delta )
                    for (int v = 0; v < 3; v++)
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                else
                    for (int v = 0; v < 3; v++)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                break;
            case ZX:
                if ( (centroid.y >= delta && axis_vals[0] != delta) || centroid.y > delta )
                    for (int v = 0; v < 3; v++)
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                else
                    for (int v = 0; v < 3; v++)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                break;
            case XY:
                if ( (centroid.z >= delta && axis_vals[0] != delta) || centroid.z > delta )
                    for (int v = 0; v < 3; v++)
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                else
                    for (int v = 0; v < 3; v++)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                break;
            }
        }


        if (f_idx_list_l.size() != 0)
        {
            node->left_ = new AABBNode;
            f_idx_list_stack.push(f_idx_list_l);
            node_stack.push(node->left_);
        }
        if (f_idx_list_r.size() != 0)
        {
            node->right_ = new AABBNode;
            f_idx_list_stack.push(f_idx_list_r);
            node_stack.push(node->right_);
        }
    }

    return head;
}

GLRENDER_INLINE void AABBTree::clearTree(AABBNode* node)
{
    if (node == NULL)
        return;
    
    clearTree(node->left_);
    clearTree(node->right_);

    delete node;
};

GLRENDER_INLINE bool AABBTree::intersectTest(AABBTree* other_tree)
{

    this->clearIntersectTest();
    other_tree->clearIntersectTest();

    std::stack<AABBNode*> node_stack;
    std::stack<glm::vec3*> axes_stack;

    node_stack.push(this->head_);
    node_stack.push(other_tree->head_);

    bool is_intersect = false;

    glm::vec3 scale_A;
    glm::quat rot_A;
    glm::vec3 trans_A;
    glm::vec3 skew_A;
    glm::vec4 persp_A;
    glm::decompose(this->obj_ptr_->modelMatrix(), scale_A, rot_A, trans_A, skew_A, persp_A);
    glm::vec3 A1 = rot_A * glm::vec3(1, 0, 0);
    glm::vec3 A2 = rot_A * glm::vec3(0, 1, 0);
    glm::vec3 A3 = rot_A * glm::vec3(0, 0, 1);
    glm::vec3 axes_this[3] {A1, A2, A3};
    axes_stack.push(axes_this);

    glm::vec3 scale_B;
    glm::quat rot_B;
    glm::vec3 trans_B;
    glm::vec3 skew_B;
    glm::vec4 persp_B;
    glm::decompose(other_tree->obj_ptr_->modelMatrix(), scale_B, rot_B, trans_B, skew_B, persp_B);
    glm::vec3 B1 = rot_B * glm::vec3(1, 0, 0);
    glm::vec3 B2 = rot_B * glm::vec3(0, 1, 0);
    glm::vec3 B3 = rot_B * glm::vec3(0, 0, 1);
    glm::vec3 axes_other[3] {B1, B2, B3};
    axes_stack.push(axes_other);

    N_v_ = 0;
    C_v_ = 0;
    num_leaf_overlap_ = 0;

    while (!node_stack.empty())
    {
        AABBNode* A = node_stack.top();
        node_stack.pop();
        glm::vec3* axis_A = axes_stack.top();
        axes_stack.pop();

        AABBNode* B = node_stack.top();
        node_stack.pop();
        glm::vec3* axis_B = axes_stack.top();
        axes_stack.pop();
        
        if (A == NULL || B == NULL)
            continue;

        if (A->left_ == NULL && A->right_ == NULL && B->left_ == NULL && B->right_ == NULL)
        {
            A = A;
        }

        N_v_ += 1;

        auto t1 = std::chrono::high_resolution_clock::now();
        bool is_box_overlap = intersectTest(A, axis_A, B, axis_B);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float, std::milli> time_overlap_test = t2 - t1;

        C_v_ += time_overlap_test.count();

        if (is_box_overlap)
        {
            if (A->left_ == NULL && A->right_ == NULL && B->left_ == NULL && B->right_ == NULL)
            {
                A->is_intersect = true;
                B->is_intersect = true;
                is_intersect = true;
                num_leaf_overlap_ += 1;
            }

            if ( (A->left_ != NULL || A->right_ != NULL) && ( A->volume() > B->volume() || (B->left_ == NULL && B->right_ == NULL) ) )
            {
                if (A->left_ != NULL)
                {
                    node_stack.push(B);
                    axes_stack.push(axis_B);
                    node_stack.push(A->left_);
                    axes_stack.push(axis_A);
                }

                if (A->right_ != NULL)
                {
                    node_stack.push(B);
                    axes_stack.push(axis_B);
                    node_stack.push(A->right_);
                    axes_stack.push(axis_A);
                }
            }
            else
            {
                if (B->left_ != NULL)
                {
                    node_stack.push(A);
                    axes_stack.push(axis_A);
                    node_stack.push(B->left_);
                    axes_stack.push(axis_B);
                }

                if (B->right_ != NULL)
                {
                    node_stack.push(A);
                    axes_stack.push(axis_A);
                    node_stack.push(B->right_);
                    axes_stack.push(axis_B);
                }
            }
        }
    }

    C_v_ /= (float) N_v_;

    other_tree->N_v_ = this->N_v_;
    other_tree->C_v_ = this->C_v_;
    other_tree->num_leaf_overlap_ = this->num_leaf_overlap_;
    
    return is_intersect;
}

//doesn't support scaled matrix yet
GLRENDER_INLINE bool AABBTree::intersectTest(AABBNode* A, glm::vec3 axis_A[3], AABBNode* B, glm::vec3 axis_B[3])
{

    glm::vec3 L; // separating axis

    for (int i = 0; i < 3; i++)
    {
        L = axis_A[i];
        L = glm::normalize(L);

        float r_A = std::abs( glm::dot(A->extent_.x * axis_A[0], L) );
        r_A += std::abs( glm::dot(A->extent_.y * axis_A[1], L) );
        r_A += std::abs( glm::dot(A->extent_.z * axis_A[2], L) );

        float r_B = std::abs( glm::dot(B->extent_.x * axis_B[0], L) );
        r_B += std::abs( glm::dot(B->extent_.y * axis_B[1], L) );
        r_B += std::abs( glm::dot(B->extent_.z * axis_B[2], L) );

        glm::vec3 T = glm::vec3(B->tree_->obj_ptr_->modelMatrix() * glm::vec4(B->center_,1)) - glm::vec3(A->tree_->obj_ptr_->modelMatrix() * glm::vec4(A->center_,1));

        if ( std::abs(glm::dot(T, L)) > (r_A + r_B) )
            return false;

        L = axis_B[i];
        L = glm::normalize(L);

        r_A = std::abs( glm::dot(A->extent_.x * axis_A[0], L) );
        r_A += std::abs( glm::dot(A->extent_.y * axis_A[1], L) );
        r_A += std::abs( glm::dot(A->extent_.z * axis_A[2], L) );

        r_B = std::abs( glm::dot(B->extent_.x * axis_B[0], L) );
        r_B += std::abs( glm::dot(B->extent_.y * axis_B[1], L) );
        r_B += std::abs( glm::dot(B->extent_.z * axis_B[2], L) );

        T = glm::vec3(B->tree_->obj_ptr_->modelMatrix() * glm::vec4(B->center_,1)) - glm::vec3(A->tree_->obj_ptr_->modelMatrix() * glm::vec4(A->center_,1));

        if ( std::abs(glm::dot(T, L)) > (r_A + r_B) )
            return false;
    }

    for (int i = 0; i < 3; i++)
    {    
        for (int j = 0; j < 3; j++)
        {
            L = glm::cross(axis_A[i], axis_B[j]);
            if (glm::length(L) == 0)
                continue;
            L = glm::normalize(L);

            float r_A = std::abs( glm::dot(A->extent_.x * axis_A[0], L) );
            r_A += std::abs( glm::dot(A->extent_.y * axis_A[1], L) );
            r_A += std::abs( glm::dot(A->extent_.z * axis_A[2], L) );

            float r_B = std::abs( glm::dot(B->extent_.x * axis_B[0], L) );
            r_B += std::abs( glm::dot(B->extent_.y * axis_B[1], L) );
            r_B += std::abs( glm::dot(B->extent_.z * axis_B[2], L) );

            glm::vec3 T = glm::vec3(B->tree_->obj_ptr_->modelMatrix() * glm::vec4(B->center_,1)) - glm::vec3(A->tree_->obj_ptr_->modelMatrix() * glm::vec4(A->center_,1));

            if ( std::abs(glm::dot(T, L)) > (r_A + r_B) )
                return false;
        }
    }

    return true;    
}

GLRENDER_INLINE void AABBTree::clearIntersectTest()
{
    std::stack<AABBNode*> node_stack;
    node_stack.push(head_);

    while (!node_stack.empty())
    {
        AABBNode* node = node_stack.top();
        node_stack.pop();

        if (node == NULL)
            continue;

        node->is_intersect = false;

        node_stack.push(node->left_);
        node_stack.push(node->right_);
    }
}

} // namespace glr
