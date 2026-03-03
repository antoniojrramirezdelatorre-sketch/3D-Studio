#pragma once

#include "geometryrender.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"

// ImGui GLFW backend callbacks (we forward GLFW events to ImGui)
extern void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
extern void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
extern void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c); // <-- IMPORTANT for typing in InputText/InputFloat

class glfwCallbackManager
{
  static OpenGLWindow* app;
  
  static void errorCallback(int error, const char* description)
  {
    if (app) app->errorCallback(error, description);
  }
  
  static void resizeCallback(GLFWwindow* window, int width, int height)
  {
    if (app) app->resizeCallback(window, width, height);
  }
  
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    // 1) Forward to ImGui first
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    
    // 2) If ImGui wants the keyboard, don't pass to app
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;
    
    if (app) ((GeometryRender*)app)->keyCallBack(key, scancode, action, mods);
  }
  
  static void charCallback(GLFWwindow* window, unsigned int c)
  {
    // This is what allows typing numbers/text in ImGui widgets (InputFloat, InputText, etc.)
    ImGui_ImplGlfw_CharCallback(window, c);
    // No app forwarding here (you don't have a char callback in GeometryRender, and we don't invent it).
  }
  
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
  {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (app) ((GeometryRender*)app)->mouseButtonCallback(button, action, mods);
  }
  
  static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
  {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (app) ((GeometryRender*)app)->cursorPositionCallback(xpos, ypos);
  }
  
public:
  static void initCallbacks(OpenGLWindow* glfwapp)
  {
    app = glfwapp;
    
    glfwSetErrorCallback(errorCallback);
    glfwSetFramebufferSizeCallback(app->window(), resizeCallback);
    
    glfwSetKeyCallback(app->window(), keyCallback);
    glfwSetCharCallback(app->window(), charCallback); //text input fix 
    glfwSetMouseButtonCallback(app->window(), mouseButtonCallback);
    glfwSetCursorPosCallback(app->window(), cursorPositionCallback);
  }
};
