#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H
#include "glr_inline.h"

#include <glr/renderBase.h>

#include <glr/shader.h>
#include <glr/texture.h>

#include <string>
#include <vector>

namespace glr {

class sceneViewer : public renderBase
{
    public:

        sceneViewer(){};

        void init();

        // Camera Stuff
        void useOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

        void usePerspective(float fovy, float aspect, float zNear, float zFar);

        void addCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir);

        void modifyCamera(std::string name, glm::vec3 eyePos, glm::vec3 lookDir, glm::vec3 upDir);

        void useCamera(std::string name);

        bool cameraExist(std::string name);

        // Lighting Stuff
        void setAmbientLight(glm::vec3 color, float I);
        
        void addDirLight(glm::vec3 dir, glm::vec3 color, float I, float spec = 1.);
        
        void addPointLight(glm::vec3 pos, glm::vec3 color, float I, float spec = 1.);
        
        void addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 color, float I, float cutoffAngle, float spec = 1.);

        
        // other stuff        
        void setSceneTransform(const glm::mat4 m);

        void drawScene();

        ~sceneViewer(){}

    private:
        glm::mat4 model{1.0f}; // (NOTE: model matrix is for whole scene)
        glm::mat4 view{1.0f}; //
        glm::mat4 proj{1.0f};

        std::vector<camera> cameraList; 
        unsigned int cameraIdx;

        glm::vec3 ambientColor{0.f};
        float ambientI{0.f};
        std::vector<dirLight> dirLightList;
        std::vector<pointLight> pointLightList;
        std::vector<spotLight> spotLightList; 
        
        void setUniforms(wavefrontObj &obj, unsigned int shapeIdx, tinyobj::material_t &mat, shader* shaderPtr);
};
}

#ifndef GLRENDER_STATIC   
    #include <glr/sceneViewer.cpp>
#endif

#endif