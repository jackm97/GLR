#include <glr/obb_tree.h>
#include <glr/obj.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <glm/gtx/matrix_decompose.hpp>

#include <Eigen/Eigen>

#include <algorithm>
#include <stack>
#include <iostream>
#include <chrono>

namespace glr
{

std::string OBBTree::obb_vs_code_(
    #include <glr/shaders/aabb.vs.h>
);

std::string OBBTree::obb_fs_code_(
    #include <glr/shaders/aabb.fs.h>
);

shader OBBTree::obb_shader_;

GLRENDER_INLINE OBBTree::OBBTree(OBJ* obj)
{
    assignObj(obj);
}

GLRENDER_INLINE void OBBTree::assignObj(OBJ* obj)
{
    this->obj_ptr_ = obj;
}

GLRENDER_INLINE void OBBTree::calcTree()
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

GLRENDER_INLINE void OBBTree::clearTree()
{
    clearTree(head_);
    glRelease();
    head_ = NULL;
    num_obb_ = 0;
    total_mem_ = 0;
    num_primitives_ = 0;
    N_v_ = 0;
    C_v_ = 0;
    num_leaf_overlap_ = 0;
}

GLRENDER_INLINE void OBBTree::draw()
{
    OBBTree::obb_shader_.use();
    for (int a = 0; a < vao_list_.size(); a++)
    {
		// draw
		glBindVertexArray(vao_list_[a]);
        std::vector<int> starts;
        std::vector<int> num_verts;
        for (int n = 0; n < 6*num_obb_; n++)
        {
            starts.push_back(4 * n);
            num_verts.push_back(4);
        }
		glMultiDrawArrays(GL_LINE_LOOP, starts.data(), num_verts.data(), 6*num_obb_);
		glBindVertexArray(0);
    }
}

GLRENDER_INLINE void OBBTree::initGLBuffers()
{
    glRelease();

    vao_list_.clear();
    vbo_list_.clear();

    initGLBuffers(head_);

    is_loaded_into_gl_ = true;
}

GLRENDER_INLINE void OBBTree::initGLBuffers(OBBNode* node)
{
    if (node==NULL)
        return;

	std::vector<float> vertex_data;
    vertex_data.reserve(4*3*num_obb_);
    unsigned int VAO;
    unsigned int VBO;

    glGenVertexArrays(1, &VAO);
    vao_list_.push_back(VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    vbo_list_.push_back(VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glm::vec3 v1, v2, v3, v4;

    std::stack<OBBNode*> node_stack;

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

        glm::vec3 center = node->center_[0] * node->axes_[0] + node->center_[1] * node->axes_[1] + node->center_[2] * node->axes_[2];
        // top
        v1 = center + node->extent_.y*node->axes_[1] + node->extent_.x*node->axes_[0] - node->extent_.z*node->axes_[2];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center + node->extent_.y*node->axes_[1] + node->extent_.x*node->axes_[0] + node->extent_.z*node->axes_[2];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center + node->extent_.y*node->axes_[1] - node->extent_.x*node->axes_[0] + node->extent_.z*node->axes_[2];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center + node->extent_.y*node->axes_[1] - node->extent_.x*node->axes_[0] - node->extent_.z*node->axes_[2];
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // bottom
        v1 = center - node->extent_.y*node->axes_[1] + node->extent_.x*node->axes_[0] - node->extent_.z*node->axes_[2];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center - node->extent_.y*node->axes_[1] + node->extent_.x*node->axes_[0] + node->extent_.z*node->axes_[2];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center - node->extent_.y*node->axes_[1] - node->extent_.x*node->axes_[0] + node->extent_.z*node->axes_[2];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center - node->extent_.y*node->axes_[1] - node->extent_.x*node->axes_[0] - node->extent_.z*node->axes_[2];
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // front
        v1 = center + node->extent_.z*node->axes_[2] + node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center + node->extent_.z*node->axes_[2] + node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center + node->extent_.z*node->axes_[2] - node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center + node->extent_.z*node->axes_[2] - node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1];
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // back
        v1 = center - node->extent_.z*node->axes_[2] + node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center - node->extent_.z*node->axes_[2] + node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center - node->extent_.z*node->axes_[2] - node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center - node->extent_.z*node->axes_[2] - node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1];
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // left
        v1 = center - node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1] + node->extent_.z*node->axes_[2];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center - node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1] + node->extent_.z*node->axes_[2];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center - node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1] - node->extent_.z*node->axes_[2];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center - node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1] - node->extent_.z*node->axes_[2];
        v4 *= scale;
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);

        // right
        v1 = center + node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1] + node->extent_.z*node->axes_[2];
        v1 *= scale;
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v2 = center + node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1] + node->extent_.z*node->axes_[2];
        v2 *= scale;
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v3 = center + node->extent_.x*node->axes_[0] + node->extent_.y*node->axes_[1] - node->extent_.z*node->axes_[2];
        v3 *= scale;
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        vertex_data.push_back(color.r); vertex_data.push_back(color.g); vertex_data.push_back(color.b);
        v4 = center + node->extent_.x*node->axes_[0] - node->extent_.y*node->axes_[1] - node->extent_.z*node->axes_[2];
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

GLRENDER_INLINE void OBBTree::glRelease()
{
	if (!is_loaded_into_gl_) return;

	glDeleteBuffers(this->vbo_list_.size(), this->vbo_list_.data());
	glDeleteVertexArrays(this->vao_list_.size(), this->vao_list_.data());

	this->vao_list_.clear();
	this->vbo_list_.clear();

    is_loaded_into_gl_ = false;
}

GLRENDER_INLINE OBBTree::~OBBTree()
{
	glRelease();
    clearTree();
}

GLRENDER_INLINE OBBNode* OBBTree::calcTree(std::vector<tinyobj::index_t*> f_idx_list)
{
    if (f_idx_list.size() == 0)
        return NULL;

    OBBNode* head = new OBBNode;

    std::stack<OBBNode*> node_stack;
    std::stack<std::vector<tinyobj::index_t*>> f_idx_list_stack;

    node_stack.push(head);
    f_idx_list_stack.push(f_idx_list);

    num_obb_ = 0;
    total_mem_ = 0;
    num_primitives_ = f_idx_list.size()/3;

    // std::cout << "\nNumber of faces: " << num_faces << std::endl;
    bool doOnce = true;
    while (!node_stack.empty())
    {
        OBBNode* node = node_stack.top();
        node_stack.pop();
        num_obb_ += 1;
        total_mem_ += sizeof(*node);
        // printf("\rNumber of AABB: %i", num_AABB);
        // fflush(stdout);

        node->tree_ = this;

        f_idx_list = f_idx_list_stack.top();
        f_idx_list_stack.pop();

        calcOBBAxes(f_idx_list, node->axes_);

        float minP[3];
        float maxP[3];

        glm::vec3 p;
        for (int i = 0; i < 3; i++)
        {
            p[i] = obj_ptr_->attrib_.vertices[3 * f_idx_list[0]->vertex_index + i];
        }
        p = glm::vec3(glm::dot(node->axes_[0],p),glm::dot(node->axes_[1],p),glm::dot(node->axes_[2],p));
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
                p = glm::vec3(glm::dot(node->axes_[0],p),glm::dot(node->axes_[1],p),glm::dot(node->axes_[2],p));
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

                tmp = glm::vec3(glm::dot(node->axes_[0],tmp),glm::dot(node->axes_[1],tmp),glm::dot(node->axes_[2],tmp));

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

                tmp = glm::vec3(glm::dot(node->axes_[0],tmp),glm::dot(node->axes_[1],tmp),glm::dot(node->axes_[2],tmp));

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
            node->left_ = new OBBNode;
            f_idx_list_stack.push(f_idx_list_l);
            node_stack.push(node->left_);
        }
        if (f_idx_list_r.size() != 0)
        {
            node->right_ = new OBBNode;
            f_idx_list_stack.push(f_idx_list_r);
            node_stack.push(node->right_);
        }
    }
    // std::cout << "\nTotal memory of AABB Tree: " << total_mem/1e6 << std::endl;

    return head;
}

GLRENDER_INLINE glm::vec3 OBBTree::calcMean(const std::vector<tinyobj::index_t*>& f_idx_list)
{
    int n = f_idx_list.size()/3;

    glm::vec3 mu(0);

    for (int i = 0; i < n; i++)
    {
        glm::vec3 p;
        for (int v = 0; v < 3; v++)
        {
            p[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 0]->vertex_index + v];
        }
        glm::vec3 q;
        for (int v = 0; v < 3; v++)
        {
            q[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 1]->vertex_index + v];
        }
        glm::vec3 r;
        for (int v = 0; v < 3; v++)
        {
            r[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 2]->vertex_index + v];
        }

        float m = glm::length(glm::cross(q-p,r-p))/2;

        if (m != 0)
            mu += 1/m * (p + q + r);
    }

    mu = 1.f/(6.f * n) * mu;

    return mu;
}

GLRENDER_INLINE void OBBTree::calcOBBAxes(const std::vector<tinyobj::index_t*>& f_idx_list, glm::vec3 axes[3])
{
    glm::vec3 mu_glm = calcMean(f_idx_list);

    Eigen::Vector3f mu(glm::value_ptr(mu_glm));
    Eigen::Matrix3f C = Eigen::Matrix3f::Zero();


    int n = f_idx_list.size()/3;

    for (int i = 0; i < n; i++)
    {
        Eigen::Vector3f p;
        for (int v = 0; v < 3; v++)
        {
            p[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 0]->vertex_index + v];
        }
        Eigen::Vector3f q;
        for (int v = 0; v < 3; v++)
        {
            q[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 1]->vertex_index + v];
        }
        Eigen::Vector3f r;
        for (int v = 0; v < 3; v++)
        {
            r[v] = this->obj_ptr_->attrib_.vertices[3 * f_idx_list[3*i + 2]->vertex_index + v];
        }


        float m = (q - p).cross(r - p).norm()/2;

        p = p.array() - mu.array();
        q = q.array() - mu.array();
        r = r.array() - mu.array();

        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                C(j,k) += 1.f/(24.f * n) * m * ( (p[j] + q[j] + r[j])*(p[k] + q[k] + r[k]) + p[j]*p[k] +q[j]*q[k] + r[j]*r[k] );
            }
        }
    }

    Eigen::Matrix3cf eigen_vecs;
    Eigen::EigenSolver<Eigen::Matrix3f> solver(C);
    eigen_vecs = solver.eigenvectors();
    

    for (int k = 0; k < 3; k++)
    {
        for (int j = 0; j < 3; j++)
        {
            axes[k][j] = eigen_vecs(j,k).real();
        }

        axes[k] = glm::normalize(axes[k]);
    }
}

GLRENDER_INLINE void OBBTree::clearTree(OBBNode* node)
{
    if (node == NULL)
        return;
    
    clearTree(node->left_);
    clearTree(node->right_);

    delete node;
};

GLRENDER_INLINE bool OBBTree::intersectTest(OBBTree* other_tree)
{

    this->clearIntersectTest();
    other_tree->clearIntersectTest();

    std::stack<OBBNode*> node_stack;
    std::stack<glm::quat*> rot_stack;

    node_stack.push(this->head_);
    node_stack.push(other_tree->head_);

    bool is_intersect = false;

    glm::vec3 scale_A;
    glm::quat rot_A;
    glm::vec3 trans_A;
    glm::vec3 skew_A;
    glm::vec4 persp_A;
    glm::decompose(this->obj_ptr_->modelMatrix(), scale_A, rot_A, trans_A, skew_A, persp_A);
    rot_stack.push(&rot_A);

    glm::vec3 scale_B;
    glm::quat rot_B;
    glm::vec3 trans_B;
    glm::vec3 skew_B;
    glm::vec4 persp_B;
    glm::decompose(other_tree->obj_ptr_->modelMatrix(), scale_B, rot_B, trans_B, skew_B, persp_B);
    rot_stack.push(&rot_B);

    N_v_ = 0;
    C_v_ = 0;
    num_leaf_overlap_ = 0;

    while (!node_stack.empty())
    {
        OBBNode* A = node_stack.top();
        node_stack.pop();
        glm::vec3 axis_A[3];
        for (int i = 0; i < 3; i++)
            axis_A[i] = A->axes_[i];
        glm::quat* A_rot_ptr = rot_stack.top();
        rot_stack.pop();
        for (int i = 0; i < 3; i++)
            axis_A[i] = (*A_rot_ptr) * axis_A[i];

        OBBNode* B = node_stack.top();
        node_stack.pop();
        glm::vec3 axis_B[3];
        for (int i = 0; i < 3; i++)
            axis_B[i] = B->axes_[i];
        glm::quat* B_rot_ptr = rot_stack.top();
        rot_stack.pop();
        for (int i = 0; i < 3; i++)
            axis_B[i] = (*B_rot_ptr) * axis_B[i];
        
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
                    rot_stack.push(B_rot_ptr);
                    node_stack.push(A->left_);
                    rot_stack.push(A_rot_ptr);
                }

                if (A->right_ != NULL)
                {
                    node_stack.push(B);
                    rot_stack.push(B_rot_ptr);
                    node_stack.push(A->right_);
                    rot_stack.push(A_rot_ptr);
                }
            }
            else
            {
                if (B->left_ != NULL)
                {
                    node_stack.push(A);
                    rot_stack.push(A_rot_ptr);
                    node_stack.push(B->left_);
                    rot_stack.push(B_rot_ptr);
                }

                if (B->right_ != NULL)
                {
                    node_stack.push(A);
                    rot_stack.push(A_rot_ptr);
                    node_stack.push(B->right_);
                    rot_stack.push(B_rot_ptr);
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
GLRENDER_INLINE bool OBBTree::intersectTest(OBBNode* A, glm::vec3 axis_A[3], OBBNode* B, glm::vec3 axis_B[3])
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

        glm::vec3 A_center = A->center_[0] * A->axes_[0] + A->center_[1] * A->axes_[1] + A->center_[2] * A->axes_[2];
        glm::vec3 B_center = B->center_[0] * B->axes_[0] + B->center_[1] * B->axes_[1] + B->center_[2] * B->axes_[2];

        glm::vec3 T = glm::vec3(B->tree_->obj_ptr_->modelMatrix() * glm::vec4(B_center,1)) - glm::vec3(A->tree_->obj_ptr_->modelMatrix() * glm::vec4(A_center,1));

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

            glm::vec3 A_center = A->center_[0] * A->axes_[0] + A->center_[1] * A->axes_[1] + A->center_[2] * A->axes_[2];
            glm::vec3 B_center = B->center_[0] * B->axes_[0] + B->center_[1] * B->axes_[1] + B->center_[2] * B->axes_[2];

            glm::vec3 T = glm::vec3(B->tree_->obj_ptr_->modelMatrix() * glm::vec4(B_center,1)) - glm::vec3(A->tree_->obj_ptr_->modelMatrix() * glm::vec4(A_center,1));

            if ( std::abs(glm::dot(T, L)) > (r_A + r_B) )
                return false;
        }
    }

    return true;    
}

GLRENDER_INLINE void OBBTree::clearIntersectTest()
{
    std::stack<OBBNode*> node_stack;
    node_stack.push(head_);

    while (!node_stack.empty())
    {
        OBBNode* node = node_stack.top();
        node_stack.pop();

        if (node == NULL)
            continue;

        node->is_intersect = false;

        node_stack.push(node->left_);
        node_stack.push(node->right_);
    }
}

} // namespace glr
