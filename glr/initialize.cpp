#include <glr/initialize.h>

#include <glad/glad.h>

#include <stdio.h>

namespace glr
{

void initialize()
{
    // Initialize OpenGL loader
    bool err = gladLoadGL() == 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize GLR library!\n");
    }
}

}

