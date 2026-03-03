#include "geometryrender.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <GLFW/glfw3.h> 
#include <map>



//texture for the object from the stb_image.h library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct VertexIdx {
    unsigned int v_idx;
    unsigned int vn_idx;
    bool operator<(const VertexIdx& other) const {
        if (v_idx != other.v_idx) return v_idx < other.v_idx;
        return vn_idx < other.vn_idx;
    }
};


using namespace std;

void GeometryRender::DrawGui()
{
    renderGUI();
}


// DESTRUCTOR and INIT

GeometryRender::~GeometryRender()
{}

void GeometryRender::initialize()
{


    glEnable(GL_DEPTH_TEST);

    program = initProgram("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    // Get uniform locations
    locModel = glGetUniformLocation(program, "M");
    locView  = glGetUniformLocation(program, "V");
    locProj  = glGetUniformLocation(program, "P");

    locLightPos = glGetUniformLocation(program, "lightPos");
    locLightColor = glGetUniformLocation(program, "lightColor");
    locAmbientColor = glGetUniformLocation(program, "ambientColor");
    locCamPos = glGetUniformLocation(program, "camPos");

    locMatAmbient = glGetUniformLocation(program, "matAmbient");
    locMatDiffuse = glGetUniformLocation(program, "matDiffuse");
    locMatSpecular = glGetUniformLocation(program, "matSpecular");
    locMatShininess = glGetUniformLocation(program, "matShininess");

    locUseTexture = glGetUniformLocation(program, "useTexture");
    locTexUnit    = glGetUniformLocation(program, "texUnit");

    // camera specs
    p0   = Vec4(0.0f, 0.0f, 2.0f, 1.0f);
    pref = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vup  = Vec4(0.0f, 1.0f, 0.0f, 0.0f);
    d    = 2.0f; 

    // Init Matrices
    resetModelMatrix();
    updateViewMatrix();
    updateProjMatrix();
    
    // Upload initial uniforms
    updateModelUniform();
    updateViewUniform();
    updateProjUniform();

    // Init Geometry buffers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
    glGenBuffers(1, &iBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
    locVertices = glGetAttribLocation(program, "vPosition");
    locNormals  = glGetAttribLocation(program, "vNormal");
    locTexCoords = glGetAttribLocation(program, "vTexCoord");

    glBindVertexArray(0);
    glUseProgram(0);
    
    loadGeometry(); 
}

// DISPLAY LOOP

void GeometryRender::display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glUseProgram(program);
  glBindVertexArray(vao);
  
  updateLightUniforms();
  glUseProgram(program);
  
  const bool useTex = (textureShow && textureID != 0);
  
  glUniform1i(locUseTexture, useTex ? 1 : 0);
  glUniform1i(locTexUnit, 0);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, useTex ? textureID : 0);
  
  glPolygonMode(GL_FRONT_AND_BACK, mWireframe ? GL_LINE : GL_FILL);
  
  if (!indices.empty()) {
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(indices.size()),
                   GL_UNSIGNED_INT,
                   BUFFER_OFFSET(0));
  }
  
  debugShader();
  
  glBindVertexArray(0);
  glUseProgram(0);

}



//GUI

void GeometryRender::renderGUI()
{

    ImGui::Begin("Project Part 2 Controls");

    // File Loading
    ImGui::Text("Model Loader");
    ImGui::Text("Current: %s", objFilePath.c_str());
    if (ImGui::Button("Open OBJ File")) {
        IGFD::FileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose OBJ File", ".obj", ".");
    }

    if (IGFD::FileDialog::Instance()->Display("ChooseFileDlgKey")) {
        if (IGFD::FileDialog::Instance()->IsOk()) {
            std::string filePathName = IGFD::FileDialog::Instance()->GetFilePathName();
            if (loadOBJ(filePathName)) {
                objFilePath = IGFD::FileDialog::Instance()->GetCurrentFileName();
            }
        }
        IGFD::FileDialog::Instance()->Close();
    }
    
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Light")) {
        ImGui::Text("Light source position");
        ImGui::PushItemWidth(100);
        ImGui::InputFloat("x", &lightPos[0], 0.5f, 1.0f, "%1.1f"); ImGui::SameLine();
        ImGui::InputFloat("y", &lightPos[1], 0.5f, 1.0f, "%1.1f"); ImGui::SameLine();
        ImGui::InputFloat("z", &lightPos[2], 0.5f, 1.0f, "%1.1f");
        ImGui::PopItemWidth();

        ImGui::Text("Light source intensity:");
        ImGui::ColorEdit3("Light", lightColor);
        
        ImGui::Text("Ambient light intensity:");
        ImGui::ColorEdit3("Ambient", ambientColor);
    }

    static ImGuiFileDialog textureDialog;

    if (ImGui::CollapsingHeader("Object Texture")) {
        ImGui::Checkbox("Show texture", &textureShow);
        ImGui::Text("Texture file: %s", textureFileName.c_str());
        
        if (ImGui::Button("Open Texture File"))
            textureDialog.OpenDialog("ChooseTexDlgKey", "Choose Texture", ".jpg,.png,.bmp,.tga", ".");
        
        if (textureDialog.Display("ChooseTexDlgKey")) {
            if (textureDialog.IsOk()) {
                std::string filePath = textureDialog.GetFilePathName();
                if (loadTexture(filePath)) {
                    textureFileName = textureDialog.GetCurrentFileName();
                }
            }
            textureDialog.Close();
        }
    }
    
    if (ImGui::CollapsingHeader("Object Material")) {
        ImGui::Text("Ambient coefficient:");
        ImGui::ColorEdit3("Ambient color", materialAmbient);
        
        ImGui::Text("Diffuse coefficient:");
        ImGui::ColorEdit3("Diffuse color", materialDiffuse);
        
        ImGui::Text("Specular coefficient:");
        ImGui::ColorEdit3("Specular color", materialSpecular);

        ImGui::SliderFloat("Shininess", &materialShininess, 1.0f, 1000.0f, "%1.0f");
    }

    // Projection Controls
    ImGui::Text("Projection Type");
    bool changed = false;
    
    if (ImGui::RadioButton("Perspective", projMode == ProjMode::Perspective)) {
        projMode = ProjMode::Perspective;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Parallel/Oblique", projMode == ProjMode::Parallel)) {
        projMode = ProjMode::Parallel;
        changed = true;
    }

    if (projMode == ProjMode::Perspective) {
        if (ImGui::SliderFloat("FOV", &fovDeg, 20.0f, 160.0f)) changed = true;
        if (ImGui::InputFloat("Far", &zFar)) changed = true;
    } else {
        if (ImGui::InputFloat("Top", &topOrtho)) changed = true;
        if (ImGui::InputFloat("Far", &zFarOrtho)) changed = true;
        if (ImGui::SliderFloat("Oblique Scale (a)", &obliqueA, 0.0f, 1.0f)) changed = true;
        if (ImGui::SliderFloat("Oblique Angle (phi)", &obliquePhiDeg, 15.0f, 75.0f)) changed = true;
    }

    if (changed) updateProjMatrix();

    ImGui::End();

}


bool GeometryRender::loadTexture(const std::string& filename)
{
  int width = 0, height = 0, nrChannels = 0;
  
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
  
  if (!data) {
    std::cerr << "Failed to load texture: " << filename
              << " (stb reason: " << stbi_failure_reason() << ")\n";
    return false;
  }
  
  if (textureID == 0) {
    glGenTextures(1, &textureID);
  }
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, textureID);
  
  GLenum format = GL_RGB;
  if (nrChannels == 1) format = GL_RED;
  else if (nrChannels == 4) format = GL_RGBA;
  
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
               format, GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glGenerateMipmap(GL_TEXTURE_2D);
  
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  textureShow = true; // para que se vea al instante si el shader/UV están bien
  
  std::cout << "Texture loaded: " << filename
            << " (" << width << "x" << height << ", channels=" << nrChannels
            << ", id=" << textureID << ")\n";
  
  return true;
}

//INPUT CALLBACKS

void GeometryRender::resizeCallback(GLFWwindow* window, int width, int height)
{
    if (height <= 0) height = 1;
    winW = width;
    winH = height;
    aspect = (float)winW / (float)winH;
    glViewport(0, 0, winW, winH);
    
    updateProjMatrix();
}

void GeometryRender::keyCallBack(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
        if (action == GLFW_PRESS) isShiftPressed = true;
        else if (action == GLFW_RELEASE) isShiftPressed = false;
    }

    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // Model transforms (Legacy Part 1)
    Mat4x4 T;
    switch (key) {
        case GLFW_KEY_LEFT:  makeRotationY(T, -10.0f); applyTransform(T); return;
        case GLFW_KEY_RIGHT: makeRotationY(T,  10.0f); applyTransform(T); return;
        case GLFW_KEY_UP:    makeRotationX(T, -10.0f); applyTransform(T); return;
        case GLFW_KEY_DOWN:  makeRotationX(T,  10.0f); applyTransform(T); return;
        case GLFW_KEY_I:     makeTranslation(T, 0.0f,  0.1f, 0.0f); applyTransform(T); return;
        case GLFW_KEY_K:     makeTranslation(T, 0.0f, -0.1f, 0.0f); applyTransform(T); return;
        case GLFW_KEY_J:     makeTranslation(T, 0.1f,  0.0f, 0.0f); applyTransform(T); return;
        case GLFW_KEY_L:     makeTranslation(T,-0.1f,  0.0f, 0.0f); applyTransform(T); return;
        case GLFW_KEY_P:     if (action == GLFW_PRESS) mWireframe = !mWireframe; return;
        case GLFW_KEY_R:     if (action == GLFW_PRESS) { resetModelMatrix(); updateModelUniform(); } return;
    }

    // Camera transforms (WASDQE)
    const float speed = 0.1f;
    switch (key) {
        case GLFW_KEY_W: moveCameraLocal(0.0f, 0.0f, -speed); break;
        case GLFW_KEY_S: moveCameraLocal(0.0f, 0.0f,  speed); break;
        case GLFW_KEY_A: moveCameraLocal(-speed, 0.0f, 0.0f); break;
        case GLFW_KEY_D: moveCameraLocal( speed, 0.0f, 0.0f); break;
        case GLFW_KEY_Q: moveCameraLocal(0.0f, -speed, 0.0f); break;
        case GLFW_KEY_E: moveCameraLocal(0.0f,  speed, 0.0f); break;
    }
}

void GeometryRender::mouseButtonCallback(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
            glfwGetCursorPos(window(), &lastMouseX, &lastMouseY);
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void GeometryRender::cursorPositionCallback(double xpos, double ypos)
{
    if (isDragging || isShiftPressed) {
        float dx = static_cast<float>(xpos - lastMouseX);
        float dy = static_cast<float>(ypos - lastMouseY);
        lastMouseX = xpos;
        lastMouseY = ypos;

        if (dx != 0.0f || dy != 0.0f) {
            const float sensitivity = 0.2f;
            rotateCamera(dx * sensitivity, dy * sensitivity);
        }
    } else {
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

// --- MATRIX LOGIC ---

void GeometryRender::moveCameraLocal(float dx, float dy, float dz)
{
    Vec3 right(matView[0], matView[1], matView[2]);
    Vec3 up(matView[4], matView[5], matView[6]);
    Vec3 back(matView[8], matView[9], matView[10]);

    Vec3 dX = mul(right, dx);
    Vec3 dY = mul(up, dy);
    Vec3 dZ = mul(back, dz);
    Vec3 move = add(add(dX, dY), dZ);

    p0.x += move.x; p0.y += move.y; p0.z += move.z;
    pref.x += move.x; pref.y += move.y; pref.z += move.z;

    updateViewMatrix();
}

void GeometryRender::rotateCamera(float dYaw, float dPitch)
{
    Vec3 viewDir = sub(xyz(pref), xyz(p0));
    float dist = length(viewDir);

    Vec3 upVec(matView[4], matView[5], matView[6]);
    Vec3 rightVec(matView[0], matView[1], matView[2]);

    float radYaw = -dYaw * 3.14159f / 180.0f;
    float radPitch = -dPitch * 3.14159f / 180.0f;

    auto rotateVec = [](Vec3 v, Vec3 k, float theta) -> Vec3 {
        float c = cos(theta);
        float s = sin(theta);
        Vec3 kxv = cross(k, v);
        float kdv = dot(k, v);
        return Vec3(
            v.x*c + kxv.x*s + k.x*kdv*(1-c),
            v.y*c + kxv.y*s + k.y*kdv*(1-c),
            v.z*c + kxv.z*s + k.z*kdv*(1-c)
        );
    };

    viewDir = rotateVec(viewDir, upVec, radYaw);
    viewDir = rotateVec(viewDir, rightVec, radPitch);
    
    viewDir = normalize(viewDir);
    pref.x = p0.x + viewDir.x * dist;
    pref.y = p0.y + viewDir.y * dist;
    pref.z = p0.z + viewDir.z * dist;

    updateViewMatrix();
}

void GeometryRender::updateViewMatrix()
{
    buildViewLookAt(matView, p0, pref, vup);
    updateViewUniform();
}

void GeometryRender::updateProjMatrix()
{
    if (projMode == ProjMode::Perspective) {
        buildPerspective(matProj, fovDeg, aspect, zNear, zFar);
    } else {
        buildParallelOblique(matProj, topOrtho, aspect, 0.0f, zFarOrtho, obliqueA, obliquePhiDeg);
    }
    updateProjUniform();
}

void GeometryRender::buildViewLookAt(Mat4x4 V, const Vec4& p0, const Vec4& pref, const Vec4& vup)
{
    Vec3 eye = xyz(p0);
    Vec3 center = xyz(pref);
    Vec3 upRef = xyz(vup);

    Vec3 f = normalize(sub(center, eye));
    Vec3 s = normalize(cross(f, upRef));
    Vec3 u = cross(s, f);

    V[0] = s.x;  V[1] = s.y;  V[2] = s.z;  V[3] = -dot(s, eye);
    V[4] = u.x;  V[5] = u.y;  V[6] = u.z;  V[7] = -dot(u, eye);
    V[8] = -f.x; V[9] = -f.y; V[10]= -f.z; V[11]= dot(f, eye);
    V[12]= 0.0f; V[13]= 0.0f; V[14]= 0.0f; V[15]= 1.0f;
}

void GeometryRender::buildPerspective(Mat4x4 P, float fovDeg, float aspect, float zNear, float zFar)
{
    for(int i=0; i<16; ++i) P[i] = 0.0f;
    float fovRad = fovDeg * 3.14159f / 180.0f;
    float top = zNear * std::tan(fovRad * 0.5f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    P[0]  = (2.0f * zNear) / (right - left);
    P[5]  = (2.0f * zNear) / (top - bottom);
    P[10] = -(zFar + zNear) / (zFar - zNear);
    P[11] = -(2.0f * zFar * zNear) / (zFar - zNear);
    P[14] = -1.0f;
}

void GeometryRender::buildParallelOblique(Mat4x4 P, float top, float aspect, float zNear, float zFar, float a, float phiDeg)
{
    float bottom = -top;
    float right  = top * aspect;
    float left   = -right;
    float phiRad = phiDeg * 3.14159f / 180.0f;

    // Shear Matrix
    Mat4x4 H; setIdentity(H);
    H[2] = a * std::cos(phiRad);
    H[6] = a * std::sin(phiRad);

    // Ortho Matrix
    Mat4x4 O; setIdentity(O);
    O[0]  = 2.0f / (right - left);
    O[5]  = 2.0f / (top - bottom);
    O[10] = -2.0f / (zFar - zNear);
    O[3]  = -(right + left) / (right - left);
    O[7]  = -(top + bottom) / (top - bottom);
    O[11] = -(zFar + zNear) / (zFar - zNear);

    multiply(P, O, H);
}

// --- HELPERS & LOADER ---

void GeometryRender::updateModelUniform() {
    glUseProgram(program);
    glUniformMatrix4fv(locModel, 1, GL_TRUE, matModel);
}

void GeometryRender::updateViewUniform() {
    glUseProgram(program);
    glUniformMatrix4fv(locView, 1, GL_TRUE, matView);
}

void GeometryRender::updateProjUniform() {
    glUseProgram(program);
    glUniformMatrix4fv(locProj, 1, GL_TRUE, matProj);
}

void GeometryRender::setIdentity(Mat4x4 M) {
    for(int i=0; i<16; ++i) M[i] = 0.0f;
    M[0]=1.0f; M[5]=1.0f; M[10]=1.0f; M[15]=1.0f;
}

void GeometryRender::multiply(Mat4x4 C, const Mat4x4 A, const Mat4x4 B) {
    Mat4x4 temp;
    for (int r=0; r<4; ++r) {
        for (int c=0; c<4; ++c) {
            float sum = 0.0f;
            for (int k=0; k<4; ++k) sum += A[r*4+k] * B[k*4+c];
            temp[r*4+c] = sum;
        }
    }
    memcpy(C, temp, sizeof(Mat4x4));
}

void GeometryRender::makeTranslation(Mat4x4 T, float dx, float dy, float dz) {
    setIdentity(T); T[3]=dx; T[7]=dy; T[11]=dz;
}
void GeometryRender::makeScale(Mat4x4 S, float sx, float sy, float sz) {
    setIdentity(S); S[0]=sx; S[5]=sy; S[10]=sz;
}
void GeometryRender::makeRotationX(Mat4x4 R, float degrees) {
    float rad = degrees * 3.14159f / 180.0f;
    float c=cos(rad), s=sin(rad);
    setIdentity(R); R[5]=c; R[6]=-s; R[9]=s; R[10]=c;
}
void GeometryRender::makeRotationY(Mat4x4 R, float degrees) {
    float rad = degrees * 3.14159f / 180.0f;
    float c=cos(rad), s=sin(rad);
    setIdentity(R); R[0]=c; R[2]=s; R[8]=-s; R[10]=c;
}
void GeometryRender::makeRotationZ(Mat4x4 R, float degrees) {
    float rad = degrees * 3.14159f / 180.0f;
    float c=cos(rad), s=sin(rad);
    setIdentity(R); R[0]=c; R[1]=-s; R[4]=s; R[5]=c;
}

void GeometryRender::resetModelMatrix() {
    setIdentity(matModel);
}

void GeometryRender::applyTransform(const Mat4x4 T) {
    Mat4x4 R;
    multiply(R, T, matModel);
    memcpy(matModel, R, sizeof(Mat4x4));
    updateModelUniform();
}

void GeometryRender::debugShader(void) const {
    GLint logSize;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize > 0) {
        char logMsg[1024];
        glGetProgramInfoLog(program, 1023, nullptr, logMsg);
        std::cerr << "Shader Log: " << logMsg << std::endl;
    }
}

void GeometryRender::loadGeometry(void)
{
    vertices.clear(); indices.clear();

    vertices.push_back(Vertex(Vec3(-0.5f, -0.75f, 0.0f), Vec3(0,0,1), Vec2(0,0)));
    vertices.push_back(Vertex(Vec3( 0.0f,  0.75f, 0.0f), Vec3(0,0,1), Vec2(0.5,1)));
    vertices.push_back(Vertex(Vec3( 0.5f, -0.75f, 0.0f), Vec3(0,0,1), Vec2(1,0)));

    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    uploadGeometryToGPU();
}

void GeometryRender::computeNormals()
{
  //reset
  for (auto& v : vertices) {
    v.normal = Vec3(0.0f, 0.0f, 0.0f);
  }

  for (size_t i = 0; i + 2 < indices.size(); i += 3) {
    unsigned int i0 = indices[i];
    unsigned int i1 = indices[i + 1];
    unsigned int i2 = indices[i + 2];
    
    if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
      continue;
    
    Vec3 p0 = vertices[i0].position;
    Vec3 p1 = vertices[i1].position;
    Vec3 p2 = vertices[i2].position;
    
    Vec3 e1 = sub(p1, p0);
    Vec3 e2 = sub(p2, p0);
    Vec3 n  = cross(e1, e2);
    
    vertices[i0].normal = add(vertices[i0].normal, n);
    vertices[i1].normal = add(vertices[i1].normal, n);
    vertices[i2].normal = add(vertices[i2].normal, n);
  }
  
  for (auto& v : vertices) {
    float len = length(v.normal);
    if (len > 1e-8f) {
      v.normal = mul(v.normal, 1.0f / len);
    } else {
      v.normal = Vec3(0.0f, 1.0f, 0.0f);
    }
  }
}


bool GeometryRender::loadOBJ(const std::string& filename)
{
    ifstream in(filename);
    if (!in) return false;

    vector<Vec3> tempPositions;
    vector<Vec3> tempNormals;
    
    // Mapa para evitar duplicar vértices idénticos
    map<VertexIdx, unsigned int> uniqueVertices;
    
    vector<Vertex> finalVertices;
    vector<unsigned int> finalIndices;

    float minX=1e9, minY=1e9, minZ=1e9, maxX=-1e9, maxY=-1e9, maxZ=-1e9;
    string line;
    bool hasNormals = false;

    while (getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream iss(line);
        string prefix; iss >> prefix;

        if (prefix == "v") {
            float x, y, z; iss >> x >> y >> z;
            tempPositions.emplace_back(x, y, z);
            if (x < minX) minX = x; if (x > maxX) maxX = x;
            if (y < minY) minY = y; if (y > maxY) maxY = y;
            if (z < minZ) minZ = z; if (z > maxZ) maxZ = z;
        } 
        else if (prefix == "vn") {
            float x, y, z; iss >> x >> y >> z;
            tempNormals.emplace_back(x, y, z);
            hasNormals = true;
        }
        else if (prefix == "f") {
            string vstr; 
            vector<VertexIdx> faceVerts;
            while (iss >> vstr) {
                size_t slash1 = vstr.find('/');
                size_t slash2 = (slash1 != string::npos) ? vstr.find('/', slash1 + 1) : string::npos;
                
                unsigned int vIdx = 0, vnIdx = 0;
                
                if (slash1 == string::npos) {
                    vIdx = stoul(vstr);
                } else if (slash2 == string::npos) {
                    vIdx = stoul(vstr.substr(0, slash1));
                } else if (slash1 + 1 == slash2) {
                    vIdx = stoul(vstr.substr(0, slash1));
                    vnIdx = stoul(vstr.substr(slash2 + 1));
                } else {
                    vIdx = stoul(vstr.substr(0, slash1));
                    vnIdx = stoul(vstr.substr(slash2 + 1));
                }
                
                // neg base
                if (vIdx > 0) vIdx--;
                if (vnIdx > 0) vnIdx--;
                
                faceVerts.push_back({vIdx, vnIdx});
            }

            if (faceVerts.size() >= 3) {
                for (size_t k = 1; k+1 < faceVerts.size(); ++k) {
                    VertexIdx tri[3] = { faceVerts[0], faceVerts[k], faceVerts[k+1] };
                    for (int i=0; i<3; ++i) {
                        // crea vertices (funciona (?))
                        if (uniqueVertices.count(tri[i]) == 0) {
                            uniqueVertices[tri[i]] = (unsigned int)finalVertices.size();
                            
                            Vertex newVert;
                            //pos
                            if (tri[i].v_idx < tempPositions.size())
                                newVert.position = tempPositions[tri[i].v_idx];
                            
                            if (hasNormals && tri[i].vn_idx < tempNormals.size()) {
                                newVert.normal = tempNormals[tri[i].vn_idx];
                            } else {
                                newVert.normal = Vec3(0,0,0);
                            }
                            newVert.texCoord = Vec2(0,0);
                            
                            finalVertices.push_back(newVert);
                        }
                        finalIndices.push_back(uniqueVertices[tri[i]]);
                    }
                }
            }
        }
    }

    if (finalVertices.empty()) return false;

    float sx = maxX - minX, sy = maxY - minY, sz = maxZ - minZ;
    float maxDim = std::max(sx, std::max(sy, sz));
    if (maxDim <= 0) maxDim = 1.0f;
    float scale = 1.0f / maxDim;
    float cx = (minX + maxX) * 0.5f, cy = (minY + maxY) * 0.5f, cz = (minZ + maxZ) * 0.5f;

    for (auto& v : finalVertices) {
        v.position.x = (v.position.x - cx) * scale;
        v.position.y = (v.position.y - cy) * scale;
        v.position.z = (v.position.z - cz) * scale;
    }

    vertices = std::move(finalVertices);
    indices = std::move(finalIndices);

    // calculate normals in case they don't exist
    if (!hasNormals) {
        computeNormals();
        std::cout << "Normals computed automatically." << std::endl;
    }
    generateTwoPartTexCoords();

    uploadGeometryToGPU();
    resetModelMatrix();
    updateModelUniform();
    return true;
}


void GeometryRender::generateTwoPartTexCoords()
{
    const float PI = 3.14159265358979323846f;

    float minY =  1e9f;
    float maxY = -1e9f;
    for (const auto& v : vertices) {
        minY = std::min(minY, v.position.y);
        maxY = std::max(maxY, v.position.y);
    }
    float height = std::max(1e-6f, maxY - minY);

    const float capFrac = 0.20f; // 20%  up/down

    for (auto& v : vertices) {
        Vec3 p = v.position;
        Vec3 n = normalize(p);

        float u = 0.5f + (atan2(n.z, n.x) / (2.0f * PI));

        // t = 0 -> bottom, 1  -> top
        float t = (p.y - minY) / height;

        bool inCap = (t < capFrac) || (t > (1.0f - capFrac));

        float vCoord;
        if (inCap) {
            //(0 top, 1 bottom)
            float yClamped = std::max(-1.0f, std::min(1.0f, n.y));
            vCoord = 0.5f - (asin(yClamped) / PI);
        } else {
            vCoord = 1.0f - t;
        }

        v.texCoord = Vec2(u, vCoord);
    }
}



void GeometryRender::uploadGeometryToGPU()
{
  glUseProgram(program);
  glBindVertexArray(vao);
  
  glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(Vertex),
               vertices.data(),
               GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices.size() * sizeof(unsigned int),
               indices.data(),
               GL_STATIC_DRAW);
  
  //offsetof for vec3
  if ((GLint)locVertices != -1) {
    glVertexAttribPointer(locVertices, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          BUFFER_OFFSET(offsetof(Vertex, position)));
    glEnableVertexAttribArray(locVertices);
  }
  
  if ((GLint)locNormals != -1) {
    glVertexAttribPointer(locNormals, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          BUFFER_OFFSET(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(locNormals);
  }
  
  if ((GLint)locTexCoords != -1) {
    glVertexAttribPointer(locTexCoords, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          BUFFER_OFFSET(offsetof(Vertex, texCoord)));
    glEnableVertexAttribArray(locTexCoords);
  }
  
  glBindVertexArray(0);
  glUseProgram(0);
}



void GeometryRender::updateLightUniforms()
{
  glUseProgram(program);
  
  glUniform3fv(locLightPos, 1, lightPos);
  glUniform3fv(locLightColor, 1, lightColor);
  glUniform3fv(locAmbientColor, 1, ambientColor);
  
  glUniform3f(locCamPos, p0.x, p0.y, p0.z);
  
  glUniform3fv(locMatAmbient, 1, materialAmbient);
  glUniform3fv(locMatDiffuse, 1, materialDiffuse);
  glUniform3fv(locMatSpecular, 1, materialSpecular);
  glUniform1f(locMatShininess, materialShininess);
}
