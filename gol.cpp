#include "gol.hpp"
#include <thread>

int main(int argc, char **argv)
{
    auto instance = render::openWindow(glm::uvec2(1920,1080), "Game Of Life", true, []()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_RESIZABLE, false);
        glfwWindowHint(GLFW_MAXIMIZED, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        window.monitor = glfwGetPrimaryMonitor();

        window.killonfailure = true;
    });
    shaderfunc::LoadCompileShader("shaders/normalfs.fs", "shaders/normalvs.vs");
    shaderfunc::createprogram("mainprogram", "normalvs", "normalfs");
    shaderfunc::createbuffers
    (
        bio("vertexdata", 1), 
        bio("matrixdata", 2),
        bio("colordata", 3)
    );
    shaderfunc::attachBuffersToProgram("mainprogram", "vertexdata", "matrixdata", "colordata");
    currentprogram = "mainprogram";
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    shaderfunc::modifyBuffer([&]()
    {
        float vertices[] = 
        {
            0.0f, 0.5f, 0.0f,
            1.0f, -0.5f, 0.0f,
            -0.0f,  -0.5f, 0.0f
        };  
        glm::mat4 matrices[] =
        {
            glm::mat4(1.0), glm::mat4(1.0), glm::mat4(1.0)
        };
        float colors[] = 
        {
            1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f
        };
        shaderfunc::writeBufferData("vertexdata", vertices, sizeof(vertices));
        shaderfunc::writeBufferData("matrixdata", matrices, sizeof(matrices));
        shaderfunc::writeBufferData("colordata", colors, sizeof(colors));
        return;
    });
    shaderfunc::bindprogram(currentprogram);

    camera::build([&]()
    {
        cam.projection = glm::perspective(glm::radians(45.0), render::getAspectRatio(), 0.1, 100.0);
        
        cam.pos = glm::vec3(0.0f, 0.0f, -2.0f);
        cam.target = glm::vec3(0.0f, 0.0f, 0.0f);
        cam.view = glm::lookAt(
            glm::vec3(cam.pos), 
            glm::vec3(cam.target), 
            glm::vec3(0.0f, 1.0f, 0.0f));
        cam.viewpoint = shaderfunc::getUniformLocation("view");
        cam.projectionpoint = shaderfunc::getUniformLocation("projection");
        return true;
    });
    functions["updatecam"] = [&]()
    {
        cam.viewpoint = shaderfunc::getUniformLocation("view");
        cam.projectionpoint = shaderfunc::getUniformLocation("projection");
        camera::setUniforms([]()
        {
            glUniformMatrix4fv(cam.viewpoint, 1, GL_FALSE, &cam.view[0][0]);
            glUniformMatrix4fv(cam.projectionpoint, 1, GL_FALSE, &cam.projection[0][0]);
        });
    };
    functions["updatecam"]();
    float timeValue = glfwGetTime();
    glClearColor(0.0,0.0,0.0, 1.0);
    while(render::isopen())
    {
        render::clear();
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
        //render::draw();
        render::update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    render::cleanup();
    std::cout << "Program exited successfully!" << std::endl;
}