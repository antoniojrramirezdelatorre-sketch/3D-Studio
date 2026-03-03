#include "geometryrender.h"
#include "glfwcallbackmanager.h"

OpenGLWindow* glfwCallbackManager::app = nullptr;

int main(int argc, char **argv)
{
    GeometryRender app("Computer Graphics Project Part 2", 1024, 768);
    glfwCallbackManager::initCallbacks(&app);
    
    app.initialize();
    app.start(); // main loop in start() openglwindow
    return 0;
}