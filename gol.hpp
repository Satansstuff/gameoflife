#ifndef _HEADER_GOL
#define _HEADER_GOL

#include <fstream>
#include <iostream>

#include <future>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.hpp"
namespace render
{
    void GLAPIENTRY MessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
    {
        if(severity == 0x826b)
            return;
        fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
                    type, severity, message );
    }

    double getAspectRatio(unsigned x = 0, unsigned y = 0)
    {
        return (double)(window.resolution.x * (x == 0) + x * (x != 0)) 
            / (window.resolution.y * (y == 0) + y * (y != 0));
    }

    bool openWindow(glm::uvec2 res, const char* title, bool fullscren, std::function<void(void)> func = nullptr)
    {
        glfwInit();
        window.glfwin = true;
        if(fullscren)
        {
            window.monitor = glfwGetPrimaryMonitor();
            auto mode = glfwGetVideoMode(window.monitor);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            window.resolution = glm::uvec2(mode->width, mode->height);
        }
        else
        {
            window.resolution = res;
            window.monitor = nullptr;
        }
        if(func)
        {
            func();
        }
        
        window.glwindow = glfwCreateWindow(
            window.resolution.x, window.resolution.y,
            window.title.c_str(), window.monitor, nullptr
        );
        glfwMakeContextCurrent(window.glwindow);
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if(err != GLEW_OK)
        {
            std::cerr << "Failed to init glew" << std::endl;
            std::cerr << err << std::endl;
            glfwTerminate();
            std::exit(-1);
        }
        glEnable(GL_DEPTH_TEST);
        // Shamelessly stolen from khronos.org
        glEnable              ( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( MessageCallback, 0 );
        //Is over now
        window.isrunning = true;
        window.setup = true;
        glViewport(0, 0,  window.resolution.x, window.resolution.y);
        return window.glwindow;
    }
    bool isopen()
    {
        return !glfwWindowShouldClose(window.glwindow) && window.isrunning;
    }
    void update()
    {
        glfwSwapBuffers(window.glwindow);
        glfwPollEvents();
    }
    void draw(std::function<void(void)> dfunc = nullptr)
    {

    }
    void clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    void cleanup()
    {
        for(auto &p : shaderprograms)
        {
            glDeleteProgram(p.second.handle);
        }
        for(auto &sh : shaders)
        {
            glDeleteShader(sh.second.id);
        }
    }
}
namespace camera
{
    bool build(std::function<bool(void)> bfunc)
    {
        return bfunc();
    }
    void update(std::function<void(void)> bfunc)
    {
        return bfunc();
    }
    void setUniforms(std::function<void()> func)
    {
        func();
    }
}
namespace proceduralgen
{
    //
    void generatesquare()
    {
        
    }
}
namespace shaderfunc
{
    /*
        TO IMPLEMENT
        glVertexAttribPointer 
            https://stackoverflow.com/questions/24876647/understanding-glvertexattribpointer

    */
    template<typename... T>
    bool LoadCompileShader(T ...args)
    {
        std::cout << "Loading and Compiling shaders" << std::endl;
        unsigned loaded = 0;
        static const std::size_t value = sizeof...(T);
        auto loadandcompile = [&](std::string name)
        {
            std::filesystem::path p(name);
            std::cout << "Loading shader " << p.filename() << " (" << loaded + 1 << " / " << value << ")"<< std::endl;
            if(std::filesystem::exists(p) ||
                std::filesystem::is_regular_file(p))              
            {
                std::ifstream t(p);
                std::string str;
                t.seekg(0, std::ios::end);   
                str.reserve(t.tellg());
                t.seekg(0, std::ios::beg);

                str.assign((
                    std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
                
                auto extension = p.extension().string();
                shaderobject obj;
                obj.name = p.stem();
                obj.code = str;
                obj.type = shadertype::SHADER_VERT * (extension.find("vs") != std::string::npos)
                + shadertype::SHADER_FRAG * (extension.find("fs") != std::string::npos) + shadertype::SHADER_GEO * (extension.find("gs") != std::string::npos)
                + shadertype::SHADER_COMP * (extension.find("comp") != std::string::npos);
                //Minimize text of shader?
                if(obj.type > 0 && shaders.find(name) == shaders.end())
                {
                    std::cout << "\tFound " << shadertype::prettyname[obj.type] << " called " << obj.name << std::endl;
                    std::cout << "\tCompiling...\n";
                    
                    auto *code = str.c_str();
                    obj.id = glCreateShader(obj.type);
                    
                    glShaderSource(obj.id, 1, &code, NULL);
                    glCompileShader(obj.id);
                    int  success;
                    char log[512];
                    glGetShaderiv(obj.id, GL_COMPILE_STATUS, &success);
                    if(!success)
                    {
                        glGetShaderInfoLog(obj.id, 512, NULL, log);
                        obj.status = std::string(log);
                        std::cout << obj.status << std::endl;
                        std::cout << "\tFailed to compile" << std::endl;
                    }
                    else
                    {
                        obj.status = "Compiled";
                        std::cout << "\tCompiled" << std::endl;
                        shaders[p.stem()] = obj;
                        loaded++;
                    }
                }
                else if(shaders.find(p.stem()) != shaders.end() && obj.id != 0)
                {
                    std::cout << "\t" << shadertype::prettyname[obj.type] << " " << p.stem() << " is already loaded" << std::endl;
                    std::cout << "\tSkipping..." << std::endl;
                }
                else
                {
                    std::cout << "\tShader " << p.stem() << " not recognized as a type" << std::endl;
                    std::cout << "\tSkipping..." << std::endl;
                }
            }
            else
            {
                std::cerr << "\tCould not find shader"
                    << std::endl << "\tskipping..." << std::endl;
            }
            return true;
        };
        (loadandcompile(std::forward<T>(args)), ...);
        std::cout << "Successfully loaded " << loaded << " shaders" << std::endl;
        return true;
    }
    template<typename... T>
    bool createprogram(std::string name, T... params)
    {
        bool success = false;
        std::cout << "Creating shaderprogram by the name of " << name << std::endl;
        std::array<std::string, sizeof...(params)> list = {params...};
        shaderprogram prog;
        prog.handle = glCreateProgram();
        constexpr size_t numshaders = sizeof...(params);
        if(prog.handle == 0)
        {
            std::cerr << "\t[GL] Failed to create program.." << std::endl;
            return window.rf(false);
        }
        if constexpr(!numshaders)
        {
            std::cout << "\t[WARN][Engine]No shaders specified for program... \n" 
            << "\tCreating program without linking" << std::endl;
            return true;
        }
        unsigned count = 0;
        /*
            Måste sätta attribs här...
        */
        auto attach = [&](std::string &shadername)
        {
            std::cout << "\tAttempting to attach shader: " << shadername << std::endl;
            auto shader = shaders.find(shadername);
            if(shader == shaders.end())
            {
                std::cerr << "\tShader " << shadername << " was not found, aborting..." << std::endl;
                success = false;
                return false;
            }
            auto shaderobj = shader->second;
            if(!shaderobj.isvalid())
            {
                std::cerr << "\tShader " << shadername << " is non-valid.. aborting" << std::endl;
                success = false;
                return false;
            }
            //std::cout << "\tFound " << shadertype::prettyname[shaderobj.type] << " named: " << shaderobj.name << ", attaching...\n";
            std::cout << "\t" << shadername << " is a valid " << shadertype::prettyname[shaderobj.type] << ", attaching...\n";
            glAttachShader(prog.handle, shaderobj.id);
            prog.shaders.push_back(shadername);
            count++;
            return true;
        };
        for(auto &n : list)
        {
            if(!attach(n)) return window.rf(false);
        }
        std::cout << "Attached " << count << " shaders to program.\nLinking." << std::endl;
        glLinkProgram(prog.handle);
        int32_t status;
        glGetProgramiv(prog.handle, GL_LINK_STATUS, &status);
        if(status == 0)
        {
            std::cerr << "\tFailed to link program" << std::endl;
            return window.rf(false);
        }
        else
        {
            std::cout << "\tProgram " << name << " linked successfully!" << std::endl;
            prog.name = name;
            shaderprograms[name] = std::move(prog);
        }
        currentprogram = name;
        return true;
    }
    void bindprogram(std::string name)
    {
        //std function istället?
        auto &program = shaderprograms[name];
        glUseProgram(program.handle);
        for(auto &buff : program.buffers)
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, buff.get().bufferindex, buff.get().bufferid);
        }
        currentprogram = name;
        //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, glbuffers[program.buffer].bind, glbuffers[program.buffer].bufferid);
    }
    template<class... T>
    void createbuffers(T... params)
    {
        std::array<bio, sizeof...(params)> buff = {params...};
        for(bio &p : buff)
        {
            glGenBuffers(1, &p.bufferid);
            glbuffers[p.buffername] = std::move(p);
        }
        return;
    }

    auto validateBuffer(glbuffer &m)
    {
        return (m.bufferid > 0);
    }
    template<typename F, typename... Args>
    auto modifyBuffer(F&& f, Args&&... args) -> typename std::result_of<F(Args...)>::type
    {
        auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return bound();
        /*
            glEnableVertexAttribArray(loc_attrib);
            glEnableVertexAttribArray(normal_attrib);
            glEnableVertexAttribArray(texcoord_attrib);
            // Set up the formats for my attributes
            glVertexAttribFormat(loc_attrib,      3, GL_FLOAT, GL_FALSE, 0);
            glVertexAttribFormat(normal_attrib,   3, GL_FLOAT, GL_FALSE, 12);
            glVertexAttribFormat(texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 24);
        */
    }

    auto& getBufferID(const std::string &name)
    {
        return glbuffers[name].bufferid;
    }
    auto& getBufferIndex(const std::string &name)
    {
        return glbuffers[name].bufferindex;
    }
    auto validateBuffer(const std::string &m)
    {
        return (glbuffers[m].bufferid > 0);
    }
    template<typename T>
    void writeBufferData(const std::string name, T *data, size_t size, uint32_t type = GL_SHADER_STORAGE_BUFFER, uint32_t method = GL_DYNAMIC_COPY)
    {
        glBindBuffer(type, glbuffers[name].bufferid);
        glBufferData(type, size, data, method);
        glBindBuffer(type, 0);
        //Fortsätt här ... 
    }

    template<typename...T>
    bool attachBuffersToProgram(std::string name, T... buffers)
    {
        static_assert(sizeof...(buffers) > 0, "No inputs to assign to program");
        if(glbuffers.size() == 0)
        {
            std::cerr << "[ERROR][attachBuffersToProgram] glbuffers is empty" << std::endl;
            return false;
        }
        auto program = shaderprograms.find(name);
        if(program == shaderprograms.end())
        {
            std::cerr << "[ERROR][attachBuffersToProgram] " << name << " is not a valid program" << std::endl;
            return false;
        }
        std::array<std::string, sizeof...(buffers)> array = {buffers...};
        program->second.buffers.clear();
        auto &pbv = program->second.buffers;
        pbv.reserve(sizeof...(buffers));
        for(auto &str : array)
        {
            auto b = glbuffers.find(str);
            if(b == glbuffers.end())
            {
                std::cerr << "[ERROR][attachBuffersToProgram] " << str << " is not a loaded buffer" << std::endl;
                pbv.clear();
                return false;
            }
            else
            {
                pbv.push_back(glbuffers[str]);
            }
        }
        bindprogram(currentprogram);
        return true;
    }
    template<typename... T>
    auto getUniformLocations(const T... params)
    {
        std::array<std::string, sizeof...(params)> array = {params...};
        static_assert(sizeof... (params) != 0, "No arguments passed to getUniformLoc");
        std::array<int, sizeof... (params)> ret;
        std::fill(ret.begin(), ret.end(), -1);
        auto program = shaderprograms[currentprogram].handle;
        if(program == -1)
        {
            std::cerr << "[ERROR][GetUniformLocations] program is invalid!" << std::endl;
            return ret;
        }
        for(uint32_t i = 0; i < sizeof...(params); i++)
        {
            int bpoint = glGetUniformLocation(program, array[i].c_str());
            if(bpoint == -1)
            {
                std::cerr << "[Warning] " << array[i] << " is not a uniform in (" << currentprogram << ")" << std::endl;
            }
            ret[i] = bpoint;
        }
        return ret;
    }
    auto getUniformLocation(const std::string name)
    {
        auto &program = shaderprograms[currentprogram];
        if(program.handle == -1)
        {
            std::cerr << "[ERROR][GetUniformLocations] program is invalid!" << std::endl;
            return -1;
        }
        int bpoint = glGetUniformLocation(program.handle, name.c_str());
        if(bpoint == -1)
        {
            std::cerr << "[Warning] " << name << " is not a uniform in (" << currentprogram << ")" << std::endl;
        }
        return bpoint;
    }
    int32_t getAttribLocation(const std::string name, std::string programname = currentprogram)
    {
        return glGetAttribLocation(shaderprograms[name].handle, name.c_str());
    }

}
#endif /* _HEADER_GOL */