#ifndef _HEADER_SHADERS
#define _HEADER_SHADERS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <functional>
#include <tuple>

namespace shadertype
{
    enum
    {
        SHADER_UNDEF = -1,
        SHADER_VERT = GL_VERTEX_SHADER,
        SHADER_FRAG = GL_FRAGMENT_SHADER,
        SHADER_COMP = GL_COMPUTE_SHADER,
        SHADER_GEO = GL_GEOMETRY_SHADER
    };
    static std::unordered_map<int, std::string> prettyname = {{GL_VERTEX_SHADER, "Vertex-shader"}, {GL_FRAGMENT_SHADER, "Fragment-shader"}, 
        {GL_COMPUTE_SHADER, "Compute-shader"}, {GL_GEOMETRY_SHADER, "Geometry-shader"}, {-1, "Unknown shader"}};
}
struct windowdata
{
    //Booleans
    bool glfwin = false;
    bool setup = false;
    bool fullscreen = false;
    bool isrunning = false;
    bool loadedshaders = 0;
    bool killonfailure = false;
    //Floating points

    //GLM
    glm::uvec2 resolution = glm::uvec2(1920, 1080);

    std::function<bool(const bool)> rf = [this](const bool val)
    {
        if(killonfailure && !val)
        {
            std::cout << "\n\nKillonfailure active.. killing" << std::endl;
            std::exit(-1);
        }
        return val;
    };
    std::function<void(void)> updatefunc = nullptr;
    
    //Strings
    std::string currprogram;
    std::string title = "Window"; 
    
    //GLFW stuff
    GLFWwindow* glwindow = nullptr;
    GLFWmonitor* monitor = nullptr;
    
} window;
struct cameraobj
{
    double fov = 60.00;
    glm::vec3 pos;
    glm::vec3 target;
    glm::mat4 projection, view;
    int projectionpoint = -1, viewpoint = -1;
}cam;
struct glbuffer
{
    uint32_t bufferid = 0;
    int32_t bufferindex = -1;
    size_t buffersize = 0;
    size_t currindex = 0;
    size_t objectsize = 0;
    std::string buffername;
    glbuffer() = default;
    glbuffer(std::string name, int32_t index)
    {
        buffername = name;
        bufferindex = index;
    }
};
struct shaderobject
{
    int32_t id = 0;
    int type = shadertype::SHADER_UNDEF;
    std::string name;
    std::string code;
    std::string status;
    inline bool isvalid()
    {
        return (type != shadertype::SHADER_UNDEF && id > 0);
    }
};
struct shaderprogram
{
    std::string name;
    std::vector<std::string> shaders;
    std::vector<std::reference_wrapper<glbuffer>> buffers;
    bool status = false;
    int32_t handle = 0;
    std::function<void(void)> drawfunc;
    inline bool isvalid()
    {
        return (handle != 0 && status);
    }
};


std::unordered_map<std::string, shaderobject> shaders;
std::unordered_map<std::string, shaderprogram> shaderprograms;
std::unordered_map<std::string, glbuffer> glbuffers;
std::unordered_map<std::string, std::function<void(void)>> functions;
std::string currentprogram = "";
using bio = glbuffer;
using datastream = char*;
#endif /* _HEADER_SHADERS */
