#pragma once

// ---------------------------------------------------------------------------
// GLAD loader
//   Language  : C/C++
//   Spec      : OpenGL  API gl=4.6  Profile=core
//   Options   : Generate loader = YES   (no WGL extension set needed)
//
//   Download  : https://glad.dav1d.de/
//     glad/glad.h  →  include/glad/glad.h
//     glad.c       →  src/glad.c
//
//   Build     : add glad.c to project, link opengl32.lib only
// ---------------------------------------------------------------------------
#include <windows.h>        // Win32 must come before glad on MSVC
#include <glad/glad.h>      // OpenGL 4.6 core function pointers

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

#include <string>

// ---------------------------------------------------------------------------
// OpenGLDC – Win32 / OpenGL 4.6 rendering context + high-level draw helpers
//
// Replaces the old MFC COpenGLDC / CGLView pair.
// Usage:
//   1. Call Initialize(hWnd) once in WM_CREATE.
//   2. Call Resize(w,h) in WM_SIZE.
//   3. Call Render() in the message loop idle path.
//   4. Call Shutdown() in WM_DESTROY.
// ---------------------------------------------------------------------------
class OpenGLDC
{
public:
    OpenGLDC();
    ~OpenGLDC();

    // Lifecycle
    bool Initialize(HWND hWnd);
    void Shutdown();
    void Resize(int w, int h);

    // Frame entry / exit
    void BeginFrame();   // wglMakeCurrent, clear
    void EndFrame();     // SwapBuffers, wglMakeCurrent(NULL)

    // Top-level render (calls BeginFrame / user scene / EndFrame)
    void Render();

    // -----------------------------------------------------------------------
    // Mouse / keyboard input – forward from WndProc
    // -----------------------------------------------------------------------
    void OnLButtonDown(int x, int y);
    void OnLButtonUp();
    void OnRButtonDown(int x, int y);
    void OnRButtonUp();
    void OnMouseMove(int x, int y);
    void OnMouseWheel(int delta);
    void OnKeyDown(int vkey);
    void OnKeyUp(int vkey);

    // -----------------------------------------------------------------------
    // Lighting & material
    // -----------------------------------------------------------------------
    void SetBackgroundColor(float r, float g, float b);
    void SetMaterialColor(float r, float g, float b);
    void SetWireframeColor(float r, float g, float b);
    void SetHighlightColor(float r, float g, float b);

    void SetLightDirection(float dx, float dy, float dz);

    void SetShading(bool shading);
    bool IsShading() const { return m_shading; }

    // -----------------------------------------------------------------------
    // Drawing utilities (immediate-mode style wrappers using VAO/VBO)
    // -----------------------------------------------------------------------
    void DrawLine(const glm::vec3& a, const glm::vec3& b);
    void DrawPolyline(const glm::vec3* pts, int count);
    void DrawPoint(const glm::vec3& pt);
    void DrawCoord();   // RGB axis indicators

    // Solid geometry (drawn with built-in shader + material color)
    void DrawSphere(const glm::vec3& center, float radius, int slices = 32, int stacks = 16);
    void DrawCylinder(const glm::vec3& base, const glm::vec3& axis, float radius, int slices = 32);
    void DrawCone(const glm::vec3& base, const glm::vec3& axis, float radius, int slices = 32);

    // Triangle mesh (normal per vertex)
    void DrawTriangle(const glm::vec3& n,
        const glm::vec3& v0,
        const glm::vec3& v1,
        const glm::vec3& v2);

    // -----------------------------------------------------------------------
    // Camera access
    // -----------------------------------------------------------------------
    Camera& GetCamera() { return m_camera; }

    // -----------------------------------------------------------------------
    // Picking (ray-cast; replaces old GL_SELECT path)
    // -----------------------------------------------------------------------
    // Returns world-space ray for pixel (x,y)
    void GetPickRay(int x, int y, glm::vec3& origin, glm::vec3& dir) const;

    bool IsInitialized() const { return m_initialized; }

private:
    // Win32 GL context
    HWND  m_hWnd = nullptr;
    HDC   m_hDC = nullptr;
    HGLRC m_hRC = nullptr;
    bool  m_initialized = false;

    // OpenGL 4.6 core-profile context helpers
    bool CreateGLContext();
    void DestroyGLContext();

    // Shader management
    bool BuildShaders();
    void DeleteShaders();

    GLuint CompileShader(GLenum type, const char* src);
    GLuint LinkProgram(GLuint vs, GLuint fs);

    // Shader programs
    GLuint m_progPhong = 0;   // lit solid rendering
    GLuint m_progFlat = 0;   // unlit line / point rendering

    // Uniform locations – Phong
    GLint m_uMVP_phong = -1;
    GLint m_uModel_phong = -1;
    GLint m_uNormalMat = -1;
    GLint m_uMatColor = -1;
    GLint m_uLightDir = -1;
    GLint m_uLightAmbient = -1;
    GLint m_uLightDiffuse = -1;
    GLint m_uShading = -1;

    // Uniform locations – Flat
    GLint m_uMVP_flat = -1;
    GLint m_uColor_flat = -1;

    // Dynamic geometry helpers (scratch VAO/VBO for immediate-style draws)
    GLuint m_scratchVAO = 0;
    GLuint m_scratchVBO = 0;
    void   UploadScratch(const float* data, int floatCount);
    void   DrawScratchLines(int vertexCount);
    void   DrawScratchPoints(int vertexCount);
    void   DrawScratchTriangles(int vertexCount);

    // Pre-built geometry (sphere/cylinder/cone as VBO)
    // Declared public so file-scope static helpers (UploadMesh) can access it
public:
    struct Mesh { GLuint vao = 0, vbo = 0, ibo = 0; int indexCount = 0; };

private:
    Mesh  m_sphereMesh;
    Mesh  m_cylinderMesh;
    Mesh  m_coneMesh;

    void BuildSphereMesh(Mesh& m, int slices, int stacks);
    void BuildCylinderMesh(Mesh& m, int slices);
    void BuildConeMesh(Mesh& m, int slices);
    void DrawMesh(const Mesh& m,
        const glm::mat4& modelMatrix);
    void FreeMesh(Mesh& m);

    // State
    Camera     m_camera;
    glm::vec3  m_colorBg = { 0.0f, 0.0f, 0.0f };
    glm::vec3  m_colorMaterial = { 0.88f, 0.69f, 0.09f };  // golden
    glm::vec3  m_colorWireframe = { 1.0f, 1.0f, 1.0f };
    glm::vec3  m_colorHighlight = { 1.0f, 0.0f, 0.0f };
    glm::vec3  m_colorDraw = { 1.0f, 1.0f, 1.0f };     // current draw color
    glm::vec3  m_lightDir = { 1.0f, 1.0f, 1.0f };
    bool       m_shading = true;

    // Input state
    bool m_lButtonDown = false;
    bool m_rButtonDown = false;
    int  m_lastMouseX = 0;
    int  m_lastMouseY = 0;
};