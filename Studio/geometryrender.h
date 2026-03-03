/*
 * Workshop 1
 * Computer Graphics course
 * Dept Computing Science, Umea University
 * Stefan Johansson, stefanj@cs.umu.se
 */
#pragma once

#include "openglwindow.h"
#include "3dstudio.h"
#include <vector>
#include <string>

class GeometryRender : public OpenGLWindow
{
public:
  
   ~GeometryRender();
    void DrawGui() override;

    template<typename... ARGS>
    GeometryRender(ARGS&&... args)
        : OpenGLWindow{ std::forward<ARGS>(args)... }
    {}

    void initialize();
    virtual void display() override;

    void keyCallBack(int key, int scancode, int action, int mods);
    // mouse callbacks
    void mouseButtonCallback(int button, int action, int mods);
    void cursorPositionCallback(double xpos, double ypos);

private:
    typedef float Mat4x4[16];
    Mat4x4 matModel;
    GLuint  locModel;

    Mat4x4 matView, matProj;
    GLuint locView, locProj;

    Vec4 p0   = Vec4(0.0f, 0.0f, 2.0f, 1.0f);
    Vec4 pref = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Vec4 vup  = Vec4(0.0f, 1.0f, 0.0f, 0.0f);
    float d   = 2.0f;

    int winW = 1024;
    int winH = 768;
    float aspect = 1024.0f / 768.0f;

    enum class ProjMode { Perspective, Parallel };
    ProjMode projMode = ProjMode::Perspective;

    // camera proyection parameters
    float fovDeg = 60.0f;
    float zNear  = 0.1f;
    float zFar   = 500.0f;
    float topOrtho = 1.0f;
    float zNearOrtho = 0.1f;
    float zFarOrtho  = 500.0f;
    float obliqueA = 0.0f;       
    float obliquePhiDeg = 45.0f; 

    float lightPos[3] = {0.0f, 0.0f, 0.0f};
    float lightColor[3] = {1.0f, 1.0f, 1.0f}; 
    float ambientColor[3] = {0.2f, 0.2f, 0.2f}; 

    float materialAmbient[3] = {0.5f, 0.5f, 0.5f}; 
    float materialDiffuse[3] = {0.5f, 0.5f, 0.5f};
    float materialSpecular[3] = {0.5f, 0.5f, 0.5f};
    float materialShininess = 1.0f;


    // TEXTURE STATE
    GLuint textureID = 0;
    GLint  locUseTexture;
    GLint  locTexUnit; 

    bool   textureShow = false;
    std::string textureFileName = "None";

    // UNIFORM LOCATIONS
    GLuint locLightPos;
    GLuint locLightColor;
    GLuint locAmbientColor;
    GLuint locCamPos;
    GLuint locMatAmbient;
    GLuint locMatDiffuse;
    GLuint locMatSpecular;
    GLuint locMatShininess;

    void updateLightUniforms();

    std::string objFilePath = "None";

    // Mouse
    bool rotating = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool isShiftPressed = false;
    bool isDragging = false;

    // Helper functions
    void renderGUI();

    void  resetModelMatrix();
    static void setIdentity(Mat4x4 M);
    static void multiply(Mat4x4 C, const Mat4x4 A, const Mat4x4 B);
    static void makeTranslation(Mat4x4 T, float dx, float dy, float dz);
    static void makeScale(Mat4x4 S, float sx, float sy, float sz);
    static void makeRotationX(Mat4x4 R, float degrees);
    static void makeRotationY(Mat4x4 R, float degrees);
    static void makeRotationZ(Mat4x4 R, float degrees);
    void  applyTransform(const Mat4x4 T);
    void  updateModelUniform();
        
    void updateViewUniform();
    void updateProjUniform();
    void updateViewMatrix();
    void updateProjMatrix();
    
    void buildViewLookAt(Mat4x4 V, const Vec4& p0, const Vec4& pref, const Vec4& vup);
    void buildPerspective(Mat4x4 P, float fovDeg, float aspect, float zNear, float zFar);
    void buildParallelOblique(Mat4x4 P, float top, float aspect, float zNear, float zFar, float a, float phiDeg);

    void moveCameraLocal(float dx, float dy, float dz);
    void rotateCamera(float dYaw, float dPitch);

    virtual void resizeCallback(GLFWwindow* window, int width, int height) override;

    GLuint program;
    GLuint vao;
    GLuint vBuffer;
    GLuint iBuffer;

    GLuint locVertices;
    GLuint locNormals;
    GLuint locTexCoords;

    std::vector<Vertex>       vertices;  //vertex for part3
    std::vector<unsigned int> indices;

    bool mWireframe = false;

    void debugShader(void) const;
    void loadGeometry(void);
    bool loadOBJ(const std::string& filename);
    void computeNormals();
    void generateTwoPartTexCoords();
    void uploadGeometryToGPU();

    bool loadTexture(const std::string& filename);
};