#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H
#include "glr_inline.h"

#include <glr/renderbase.h>

#include <glr/shader.h>
#include <glr/texture.h>

#include <string>
#include <vector>

namespace glr {

struct camera;
struct dirLight;
struct pointLight;
struct spotLight;

class sceneViewer : public renderBase
{
    public:

        sceneViewer(){};

        void init();

        // Camera Stuff
        void useOrtho(float left, float right, float bottom, float top, float z_near, float z_far);

        void usePerspective(float fovy, float aspect, float z_near, float z_far);

        void addCamera(std::string name, glm::vec3 eye_pos, glm::vec3 look_dir, glm::vec3 up_dir);

        camera* getCamera(std::string name);

        camera* getActiveCamera();

        std::vector<camera*> cameraList();

        void useCamera(std::string name);

        // Lighting Stuff
        void setAmbientLight(glm::vec3 color, float I);
        
        void addDirLight(glm::vec3 dir, glm::vec3 color, float I, float spec = 1.);
        
        void addPointLight(glm::vec3 pos, glm::vec3 color, float I, float spec = 1.);
        
        void addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float I, float cutoff_angle, float spec = 1.);

        
        // other stuff
        glm::mat4 modelMatrix();

        void modelMatrix(const glm::mat4 m);

        void drawScene();

        ~sceneViewer(){}

    private:
        glm::mat4 model_{1.0f}; // (NOTE: model matrix is for whole scene)
        glm::mat4 view_{1.0f}; //
        glm::mat4 proj_{1.0f};

        std::vector<camera> camera_list_; 
        unsigned int camera_idx_;

        glm::vec3 ambient_color_{0.f};
        float ambient_I_{0.f};
        std::vector<dirLight> dir_light_list_;
        std::vector<pointLight> point_light_list_;
        std::vector<spotLight> spotlight_list_; 
        
        void setUniforms();

        void setupDefaultShader();
};

struct camera
{
    public:
        std::string name;

        glm::vec3 pos;

        glm::vec3 dir;

        glm::vec3 up;
};

struct dirLight
{
    public:
        glm::vec3 dir;

        glm::vec3 color;

        float I;

        float spec;
};

struct pointLight
{
    public:
        glm::vec3 pos;

        glm::vec3 color;

        float I;

        float spec;
};

struct spotLight
{
    public:
        glm::vec3 pos;

        glm::vec3 dir;

        glm::vec3 color;

        float I;

        float spec;

        float cutoff_angle;
};
}

#ifndef GLRENDER_STATIC   
    #include <glr/sceneviewer.cpp>
#endif

#endif