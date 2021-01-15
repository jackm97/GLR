#ifndef GLR_TYPEDEFS_H
#define GLR_TYPEDEFS_H

#define TINYOBJ_CUSTOM_NAMESPACE glr
#include <glr/tinyobjloader/tiny_obj_loader.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace glr {

class shader;
class texture;
class renderBase;

class wavefrontObj
{
    public:
        std::string objPath = "", baseDir = "";

        std::string name = "";
        
        std::vector<unsigned int> VAOList;
        
        tinyobj::attrib_t attrib; // see tinyobj docs
        std::vector<tinyobj::shape_t> shapes; // see tinyobj docs
        std::vector<tinyobj::material_t> materials; // see tinyobj docs

        glm::mat4 modelMatrix{1.0f};

        std::vector<glm::vec3> shapeCenters;

        friend renderBase;

    private:
        std::vector<bool> noUVMap;

        // This vector has same size as shapes
        // and each entry is a pointer to a 
        // glr::shader object
        std::vector<shader*> shaderPtrs;

        // works similar to shaderPtrs
        std::vector<texture*> texturePtrs;

        bool isLoadedIntoGL = false;
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

        float cutOffAngle;
};

} // namespace glr

#endif