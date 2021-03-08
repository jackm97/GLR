#include <glr/initialize.h>

#include <glr/aabb_tree.h>

#ifdef GLRENDER_STATIC
#   include <glad/glad.h>
#endif

#include <stdio.h>

namespace glr
{

void initialize()
{

#ifdef GLRENDER_STATIC
    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize GLR library!\n");
    }
#endif

    AABBTree::aabb_shader_ = shader(AABBTree::aabb_vs_code_.c_str(), AABBTree::aabb_fs_code_.c_str(), RAW_CODE);
}

void cleanup()
{
    AABBTree::aabb_shader_.glRelease();
}

}

