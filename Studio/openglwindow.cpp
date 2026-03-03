#include "openglwindow.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

OpenGLWindow::OpenGLWindow(std::string title, int width, int height)
{
    if (!glfwInit()) {
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    windowWidth  = width;
    windowHeight = height;

    glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!glfwWindow) {
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();

    (void)glGetError();

    if (glewError != GLEW_OK) {
        std::exit(EXIT_FAILURE);
    }

    glPointSize(5.0f);
    glLineWidth(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, false);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

OpenGLWindow::~OpenGLWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (glfwWindow) {
        glfwDestroyWindow(glfwWindow);
        glfwWindow = nullptr;
    }
    glfwTerminate();
}

bool OpenGLWindow::checkOpenGLError() const
{
    bool foundError = false;
    GLenum glError = glGetError();
    while (glError != GL_NO_ERROR) {
        foundError = true;
        glError = glGetError();
    }
    return foundError;
}

int OpenGLWindow::width() const
{
    return windowWidth;
}

int OpenGLWindow::height() const
{
    return windowHeight;
}

GLFWwindow* OpenGLWindow::window() const
{
    return glfwWindow;
}

void OpenGLWindow::reshape(const int width, const int height) const
{
    glViewport(0, 0, width, height);
}

void OpenGLWindow::resizeCallback(GLFWwindow* , int width, int height)
{
    if (height <= 0) height = 1;
    windowWidth = width;
    windowHeight = height;
    reshape(width, height);
}

void OpenGLWindow::errorCallback(int , const char* )
{
}

std::string OpenGLWindow::readShaderSource(const std::string shaderFile) const
{
    std::ifstream fs(shaderFile, std::ios::in);
    if (!fs) return {};

    std::ostringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

GLuint OpenGLWindow::initProgram(const std::string vShaderFile, const std::string fShaderFile) const
{
    struct ShaderInfo {
        std::string filename;
        GLenum type;
    };

    ShaderInfo shaders[2] = {
        { vShaderFile, GL_VERTEX_SHADER },
        { fShaderFile, GL_FRAGMENT_SHADER }
    };

    GLuint program = glCreateProgram();
    if (!program) {
        std::exit(EXIT_FAILURE);
    }

    std::vector<GLuint> compiledShaders;
    compiledShaders.reserve(2);

    for (const auto& sh : shaders) {
        std::string source = readShaderSource(sh.filename);
        if (source.empty()) {
            std::exit(EXIT_FAILURE);
        }

        GLuint shader = glCreateShader(sh.type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint logSize = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            std::string log;
            log.resize((logSize > 1) ? (size_t)logSize : 1);
            glGetShaderInfoLog(shader, logSize, nullptr, &log[0]);
            std::exit(EXIT_FAILURE);
        }

        glAttachShader(program, shader);
        compiledShaders.push_back(shader);
    }

    glLinkProgram(program);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::exit(EXIT_FAILURE);
    }

    for (GLuint sh : compiledShaders) {
        glDetachShader(program, sh);
        glDeleteShader(sh);
    }

    return program;
}

void OpenGLWindow::DrawGui()
{
}

void OpenGLWindow::start()
{
    while (!glfwWindowShouldClose(glfwWindow)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawGui();
        display();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(glfwWindow);
    }
}

void OpenGLWindow::displayNow()
{
    if (glfwGetCurrentContext() == nullptr)
        return;

    display();
}