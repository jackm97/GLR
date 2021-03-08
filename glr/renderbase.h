#ifndef RENDERBASE_H
#define RENDERBASE_H
#include "glr_inline.h"

#include <glr/shader.h>
#include <glr/texture.h>
#include <glr/obj.h>

#include <string>
#include <vector>

namespace glr {

class renderBase
{
    public:

        renderBase(){};

        virtual void init() = 0;

        // obj stuff

        void addOBJ(std::string obj_path, std::string base_dir, std::string obj_name, bool calc_normals=false, bool flip_normals=false);

        OBJ* getOBJ(std::string obj_name);

        std::vector<OBJ*> OBJList();

        void deleteOBJ(std::string obj_name);

        
        // shader stuff
        
        void addShader(std::string vert_shader_info, std::string frag_shader_info, std::string shader_name, shaderLoadType load_type=PATH_TO_CODE);

        shader* getShader(std::string shader_name);

        std::vector<shader*> shaderList();

        void deleteShader(std::string shader_name);

        
        // texture stuff
        
        void addTexture(std::string texture_path, std::string texture_name);
        
        void addTexture(int width, int height, std::string texture_name);

        texture* getTexture(std::string texture_name);

        std::vector<texture*> textureList();

        void deleteTexture(std::string texture_name);

        // draw

        virtual void drawScene() = 0;

        // cleanup stuff   

        void cleanup();

        ~renderBase();

    protected:
        bool is_init_ = false; // so we don't initialize the renderer more than once

        std::vector<OBJ*> obj_list_;

        std::vector<shader*> shaders_;
        const int MAX_SHADER_COUNT = 100;            
        std::vector<texture*> textures_;
        const int MAX_TEXTURE_COUNT = 100; 

        bool OBJExist(std::string name);

        bool shaderExist(std::string name);

        virtual void setupDefaultShader() = 0;

        bool textureExist(std::string name);
        
        virtual void setupEmptyTexture();

        virtual void setUniforms() = 0;

};

} // namespace glr

#ifndef GLRENDER_STATIC   
    #include <glr/renderbase.cpp>
#endif

#endif