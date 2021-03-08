#include <glr/aabb_tree.h>
#include <glr/obj.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <algorithm>
#include <stack>
#include <iostream>

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
    unsigned int VAO;
    unsigned int VBO;

    glGenVertexArrays(1, &VAO);
    vao_list_.push_back(VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    vbo_list_.push_back(VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    vertex_data.clear();

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

        // top
        v1 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(node->extent_.x, 0, -node->extent_.z);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(node->extent_.x, 0, node->extent_.z);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, node->extent_.z);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(0,node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, -node->extent_.z);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        // bottom
        v1 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(node->extent_.x, 0, -node->extent_.z);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(node->extent_.x, 0, node->extent_.z);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, node->extent_.z);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(0,-node->extent_.y,0) + glm::vec3(-node->extent_.x, 0, -node->extent_.z);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        // front
        v1 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(node->extent_.x, -node->extent_.y, 0);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(node->extent_.x, node->extent_.y, 0);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(-node->extent_.x, node->extent_.y, 0);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(0,0,node->extent_.z) + glm::vec3(-node->extent_.x, -node->extent_.y, 0);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        // back
        v1 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(node->extent_.x, -node->extent_.y, 0);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(node->extent_.x, node->extent_.y, 0);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(-node->extent_.x, node->extent_.y, 0);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(0,0,-node->extent_.z) + glm::vec3(-node->extent_.x, -node->extent_.y, 0);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        // left
        v1 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, node->extent_.z);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, node->extent_.z);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, -node->extent_.z);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(-node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, -node->extent_.z);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        // right
        v1 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, node->extent_.z);
        vertex_data.push_back(v1.x); vertex_data.push_back(v1.y); vertex_data.push_back(v1.z);
        v2 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, node->extent_.z);
        vertex_data.push_back(v2.x); vertex_data.push_back(v2.y); vertex_data.push_back(v2.z);
        v3 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, node->extent_.y, -node->extent_.z);
        vertex_data.push_back(v3.x); vertex_data.push_back(v3.y); vertex_data.push_back(v3.z);
        v4 = node->center_ + glm::vec3(node->extent_.x,0,0) + glm::vec3(0, -node->extent_.y, -node->extent_.z);
        vertex_data.push_back(v4.x); vertex_data.push_back(v4.y); vertex_data.push_back(v4.z);

        if (node->left_ != NULL)
            node_stack.push(node->left_);
        if (node->right_ != NULL)
            node_stack.push(node->right_);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), vertex_data.data(), GL_DYNAMIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

    head->f_idx_list_ = f_idx_list;

    std::stack<AABBNode*> node_stack;

    node_stack.push(head);

    num_aabb_ = 0;
    float total_mem = 0;
    int num_faces = f_idx_list.size()/3;

    // std::cout << "\nNumber of faces: " << num_faces << std::endl;
    while (!node_stack.empty())
    {
        AABBNode* node = node_stack.top();
        node_stack.pop();
        num_aabb_ += 1;
        total_mem += sizeof(*node);
        // printf("\rNumber of AABB: %i", num_AABB);
        // fflush(stdout);

        f_idx_list = node->f_idx_list_;

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
            maxP[i] = minP[i];
        }

        int f_num = f_idx_list.size()/3;
        for (int f = 0; f < f_num; f++)
        {
            for (int v = 0; v < 3; v++)
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
        delta = axis_vals[axis_vals.size()/2 - 1];
        if (axis_vals[axis_vals.size() - 1] == delta)
            continue;

        std::vector<tinyobj::index_t*> f_idx_list_l;
        std::vector<tinyobj::index_t*> f_idx_list_r;
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
                if (centroid.x > delta)
                    for (int v = 0; v < 3; v++)
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                else
                    for (int v = 0; v < 3; v++)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                break;
            case ZX:
                if (centroid.y > delta)
                    for (int v = 0; v < 3; v++)
                        f_idx_list_r.push_back(f_idx_list[3*f + v]);
                else
                    for (int v = 0; v < 3; v++)
                        f_idx_list_l.push_back(f_idx_list[3*f + v]);
                break;
            case XY:
                if (centroid.z > delta)
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
            node->left_->f_idx_list_ = f_idx_list_l;
            node_stack.push(node->left_);
        }
        if (f_idx_list_r.size() != 0)
        {
            node->right_ = new AABBNode;
            node->right_->f_idx_list_ = f_idx_list_r;
            node_stack.push(node->right_);
        }
    }
    // std::cout << "\nTotal memory of AABB Tree: " << total_mem/1e6 << std::endl;

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

} // namespace glr
