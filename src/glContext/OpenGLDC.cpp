#include "OpenGLDC.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

// ============================================================================
// GLSL sources  (OpenGL 4.6 core profile, GLSL 460)
// ============================================================================

// ---------- Phong vertex shader ----------------------------------------------
static const char* kVS_Phong = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMat;

out vec3 vNormal;
out vec3 vFragPos;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos    = worldPos.xyz;
    vNormal     = normalize(uNormalMat * aNormal);
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

// ---------- Phong fragment shader -------------------------------------------
static const char* kFS_Phong = R"(
#version 460 core

in vec3 vNormal;
in vec3 vFragPos;

uniform vec3  uMatColor;
uniform vec3  uLightDir;
uniform vec3  uLightAmbient;
uniform vec3  uLightDiffuse;
uniform bool  uShading;

out vec4 FragColor;

void main()
{
    vec3 color;
    if (uShading)
    {
        vec3  L    = normalize(uLightDir);
        float diff = max(dot(vNormal, L), 0.0);
        vec3  amb  = uLightAmbient * uMatColor;
        vec3  dif  = uLightDiffuse * uMatColor * diff;
        color = amb + dif;
    }
    else
    {
        color = uMatColor;
    }
    FragColor = vec4(color, 1.0);
}
)";

// ---------- Flat vertex shader (lines / points) ------------------------------
static const char* kVS_Flat = R"(
#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

// ---------- Flat fragment shader ---------------------------------------------
static const char* kFS_Flat = R"(
#version 460 core

uniform vec3 uColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}
)";

// ============================================================================
// Construction / Destruction
// ============================================================================

OpenGLDC::OpenGLDC() = default;

OpenGLDC::~OpenGLDC()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

bool OpenGLDC::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
    m_hDC = ::GetDC(hWnd);
    if (!m_hDC) return false;

    // CreateGLContext() handles both gladLoadWGL() and gladLoadGL() internally
    if (!CreateGLContext()) return false;

    if (!BuildShaders()) return false;

    // ── Scratch VAO/VBO (position-only 3-float, for lines and points) ───────
    glGenVertexArrays(1, &m_scratchVAO);
    glGenBuffers(1, &m_scratchVBO);
    glBindVertexArray(m_scratchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_scratchVBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);

    // ── Pre-built solid meshes ───────────────────────────────────────────────
    BuildSphereMesh(m_sphereMesh, 32, 16);
    BuildCylinderMesh(m_cylinderMesh, 32);
    BuildConeMesh(m_coneMesh, 32);

    // ── Global GL state ──────────────────────────────────────────────────────
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glPointSize(3.0f);

    m_camera.Init();
    m_initialized = true;
    return true;
}

void OpenGLDC::Shutdown()
{
    if (!m_initialized) return;


    FreeMesh(m_sphereMesh);
    FreeMesh(m_cylinderMesh);
    FreeMesh(m_coneMesh);

    glDeleteVertexArrays(1, &m_scratchVAO); m_scratchVAO = 0;
    glDeleteBuffers(1, &m_scratchVBO); m_scratchVBO = 0;

    DeleteShaders();
    DestroyGLContext();

    m_initialized = false;
}

void OpenGLDC::Resize(int w, int h)
{
    if (!m_initialized) return;
  
    m_camera.SetScreen(w, h);
}

// ============================================================================
// Frame
// ============================================================================

void OpenGLDC::BeginFrame()
{ 
    m_camera.Projection(); 
    glClearColor(m_colorBg.r, m_colorBg.g, m_colorBg.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
}

void OpenGLDC::EndFrame()
{
    glFlush();
    SwapBuffers(m_hDC);
}

void OpenGLDC::Render()
{
    if (!m_initialized) return;
    BeginFrame();
  
    // Default scene: coordinate axes.
    // Call Draw* methods between BeginFrame/EndFrame to add geometry.

    DrawLine(glm::vec3(60, 60, 60), glm::vec3(500, 500, 500));
    DrawCoord();

    EndFrame();
}

// ============================================================================
// Input
// ============================================================================

void OpenGLDC::OnLButtonDown(int x, int y)
{
    m_lButtonDown = true;
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void OpenGLDC::OnLButtonUp() { m_lButtonDown = false; }

void OpenGLDC::OnRButtonDown(int x, int y)
{
    m_rButtonDown = true;
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void OpenGLDC::OnRButtonUp() { m_rButtonDown = false; }

void OpenGLDC::OnMouseMove(int x, int y)
{
    int dx = x - m_lastMouseX;
    int dy = y - m_lastMouseY;
    m_lastMouseX = x;
    m_lastMouseY = y;

    if (m_lButtonDown) m_camera.OnOrbit(dx, dy);  // left-drag  → orbit
    if (m_rButtonDown) m_camera.OnPan(dx, dy);  // right-drag → pan
}

void OpenGLDC::OnMouseWheel(int delta) { m_camera.OnZoom(delta); }

void OpenGLDC::OnKeyDown(int vkey)
{
    switch (vkey)
    {
    case 'W': m_camera.TurnUp(2.0); break;
    case 'S': m_camera.TurnDown(2.0); break;
    case 'A': m_camera.TurnLeft(2.0); break;
    case 'D': m_camera.TurnRight(2.0); break;

        // Numpad view presets (Blender-style)
    case VK_NUMPAD1: m_camera.SetViewType(ViewType::Front);        break;
    case VK_NUMPAD3: m_camera.SetViewType(ViewType::Right);        break;
    case VK_NUMPAD7: m_camera.SetViewType(ViewType::Top);          break;
    case VK_NUMPAD5: m_camera.SetViewType(ViewType::NE_Isometric); break;
    }
}

void OpenGLDC::OnKeyUp(int /*vkey*/) {}

// ============================================================================
// Color / material setters
// ============================================================================

void OpenGLDC::SetBackgroundColor(float r, float g, float b) { m_colorBg = { r,g,b }; }
void OpenGLDC::SetMaterialColor(float r, float g, float b) { m_colorMaterial = { r,g,b }; }
void OpenGLDC::SetWireframeColor(float r, float g, float b) { m_colorWireframe = { r,g,b }; }
void OpenGLDC::SetHighlightColor(float r, float g, float b) { m_colorHighlight = { r,g,b }; }
void OpenGLDC::SetShading(bool s) { m_shading = s; }

void OpenGLDC::SetLightDirection(float dx, float dy, float dz)
{
    m_lightDir = glm::normalize(glm::vec3(dx, dy, dz));
}

// ============================================================================
// Picking
// ============================================================================

void OpenGLDC::GetPickRay(int x, int y, glm::vec3& origin, glm::vec3& dir) const
{
    m_camera.GetPickRay(x, y, origin, dir);
}

// ============================================================================
// Internal: scratch VBO helpers (position-only, 3 floats / vertex)
// ============================================================================

void OpenGLDC::UploadScratch(const float* data, int floatCount)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_scratchVBO);
    glBufferData(GL_ARRAY_BUFFER,
        floatCount * sizeof(float),
        data,
        GL_DYNAMIC_DRAW);
}

void OpenGLDC::DrawScratchLines(int vertexCount)
{
    glBindVertexArray(m_scratchVAO);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
}

void OpenGLDC::DrawScratchPoints(int vertexCount)
{
    glBindVertexArray(m_scratchVAO);
    glDrawArrays(GL_POINTS, 0, vertexCount);
    glBindVertexArray(0);
}

void OpenGLDC::DrawScratchTriangles(int vertexCount)
{
    glBindVertexArray(m_scratchVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

// ============================================================================
// Flat-shader helper
// ============================================================================

static void BindFlatProgram(GLuint prog,
    const glm::mat4& mvp, const glm::vec3& color,
    GLint locMVP, GLint locColor)
{
    glUseProgram(prog);
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3fv(locColor, 1, glm::value_ptr(color));
}

// ============================================================================
// Public draw utilities
// ============================================================================

void OpenGLDC::DrawLine(const glm::vec3& a, const glm::vec3& b)
{
    BindFlatProgram(m_progFlat, m_camera.GetViewProjMatrix(),
        m_colorDraw, m_uMVP_flat, m_uColor_flat);
    float v[6] = { a.x,a.y,a.z, b.x,b.y,b.z };
    UploadScratch(v, 6);
    DrawScratchLines(2);
}

void OpenGLDC::DrawPolyline(const glm::vec3* pts, int count)
{
    if (count < 2) return;
    BindFlatProgram(m_progFlat, m_camera.GetViewProjMatrix(),
        m_colorDraw, m_uMVP_flat, m_uColor_flat);

    std::vector<float> v;
    v.reserve(count * 3);
    for (int i = 0; i < count; ++i)
    {
        v.push_back(pts[i].x);
        v.push_back(pts[i].y);
        v.push_back(pts[i].z);
    }
    UploadScratch(v.data(), (int)v.size());
    glBindVertexArray(m_scratchVAO);
    glDrawArrays(GL_LINE_STRIP, 0, count);
    glBindVertexArray(0);
}

void OpenGLDC::DrawPoint(const glm::vec3& pt)
{
    BindFlatProgram(m_progFlat, m_camera.GetViewProjMatrix(),
        m_colorDraw, m_uMVP_flat, m_uColor_flat);
    float v[3] = { pt.x, pt.y, pt.z };
    UploadScratch(v, 3);
    DrawScratchPoints(1);
}

void OpenGLDC::DrawCoord()
{
    double w, h;
    m_camera.GetViewRect(w, h);
    float len = static_cast<float>(std::min(w, h) * 0.2);

    const glm::vec3 o(0.0f);
    m_colorDraw = { 1,0,0 }; DrawLine(o, { len, 0,   0 });    // X – red
    m_colorDraw = { 0,1,0 }; DrawLine(o, { 0,   len, 0 });    // Y – green
    m_colorDraw = { 0,0,1 }; DrawLine(o, { 0,   0,   len });  // Z – blue
    m_colorDraw = m_colorWireframe;                           // restore
}

// ============================================================================
// DrawTriangle – single ad-hoc triangle with Phong shading
// ============================================================================

void OpenGLDC::DrawTriangle(const glm::vec3& n,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2)
{
    const glm::mat4 model = glm::mat4(1.0f);
    const glm::mat4 mvp = m_camera.GetViewProjMatrix() * model;
    const glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(model)));

    glUseProgram(m_progPhong);
    glUniformMatrix4fv(m_uMVP_phong, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(m_uModel_phong, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(m_uNormalMat, 1, GL_FALSE, glm::value_ptr(nm));
    glUniform3fv(m_uMatColor, 1, glm::value_ptr(m_colorMaterial));
    glUniform3fv(m_uLightDir, 1, glm::value_ptr(m_lightDir));
    glUniform3f(m_uLightAmbient, 0.75f, 0.75f, 0.75f);
    glUniform3f(m_uLightDiffuse, 1.0f, 1.0f, 1.0f);
    glUniform1i(m_uShading, m_shading ? 1 : 0);

    // Interleaved: pos(3) | normal(3) per vertex
    float verts[] = {
        v0.x, v0.y, v0.z,  n.x, n.y, n.z,
        v1.x, v1.y, v1.z,  n.x, n.y, n.z,
        v2.x, v2.y, v2.z,  n.x, n.y, n.z,
    };

    // Temporary VAO/VBO with pos+normal attribs
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)(3 * sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

// ============================================================================
// Phong mesh draw
// ============================================================================

void OpenGLDC::DrawMesh(const Mesh& m, const glm::mat4& model)
{
    if (m.vao == 0) return;

    const glm::mat4 mvp = m_camera.GetViewProjMatrix() * model;
    const glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(model)));

    glUseProgram(m_progPhong);
    glUniformMatrix4fv(m_uMVP_phong, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(m_uModel_phong, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(m_uNormalMat, 1, GL_FALSE, glm::value_ptr(nm));
    glUniform3fv(m_uMatColor, 1, glm::value_ptr(m_colorMaterial));
    glUniform3fv(m_uLightDir, 1, glm::value_ptr(m_lightDir));
    glUniform3f(m_uLightAmbient, 0.75f, 0.75f, 0.75f);
    glUniform3f(m_uLightDiffuse, 1.0f, 1.0f, 1.0f);
    glUniform1i(m_uShading, m_shading ? 1 : 0);

    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ============================================================================
// Solid shape wrappers
// ============================================================================

void OpenGLDC::DrawSphere(const glm::vec3& center, float radius,
    int /*slices*/, int /*stacks*/)
{
    DrawMesh(m_sphereMesh,
        glm::scale(glm::translate(glm::mat4(1.0f), center),
            glm::vec3(radius)));
}

// Helper: build a model matrix that aligns +Y axis to `axis` and scales
static glm::mat4 AlignYToAxis(const glm::vec3& base,
    const glm::vec3& axis,
    float radius)
{
    float     len = glm::length(axis);
    glm::vec3 dir = glm::normalize(axis);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), base);
    const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
    float cosA = glm::dot(yAxis, dir);

    if (cosA < -0.9999f)
        model = glm::rotate(model, glm::pi<float>(), glm::vec3(1, 0, 0));
    else if (cosA < 0.9999f)
        model = glm::rotate(model,
            std::acos(cosA),
            glm::normalize(glm::cross(yAxis, dir)));

    return glm::scale(model, glm::vec3(radius, len, radius));
}

void OpenGLDC::DrawCylinder(const glm::vec3& base, const glm::vec3& axis,
    float radius, int /*slices*/)
{
    DrawMesh(m_cylinderMesh, AlignYToAxis(base, axis, radius));
}

void OpenGLDC::DrawCone(const glm::vec3& base, const glm::vec3& axis,
    float radius, int /*slices*/)
{
    DrawMesh(m_coneMesh, AlignYToAxis(base, axis, radius));
}

// ============================================================================
// Mesh builders  (interleaved pos[3] | normal[3], indexed GL_TRIANGLES)
// ============================================================================

// Upload helper shared by all three builders
static void UploadMesh(OpenGLDC::Mesh& m,
    const std::vector<float>& verts,
    const std::vector<unsigned>& idx)
{
    m.indexCount = (int)idx.size();

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ibo);
    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(verts.size() * sizeof(float)),
        verts.data(), GL_STATIC_DRAW);

    // location 0: position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0);
    // location 1: normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        (GLsizeiptr)(idx.size() * sizeof(unsigned)),
        idx.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void OpenGLDC::BuildSphereMesh(Mesh& m, int slices, int stacks)
{
    std::vector<float>    verts;
    std::vector<unsigned> idx;

    for (int j = 0; j <= stacks; ++j)
    {
        float phi = glm::pi<float>() * j / stacks;
        for (int i = 0; i <= slices; ++i)
        {
            float theta = 2.0f * glm::pi<float>() * i / slices;
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            verts.insert(verts.end(), { x,y,z, x,y,z }); // pos == normal on unit sphere
        }
    }
    for (int j = 0; j < stacks; ++j)
        for (int i = 0; i < slices; ++i)
        {
            unsigned a = j * (slices + 1) + i, b = a + 1,
                c = (j + 1) * (slices + 1) + i, d = c + 1;
            idx.insert(idx.end(), { a,c,b, b,c,d });
        }

    UploadMesh(m, verts, idx);
}

void OpenGLDC::BuildCylinderMesh(Mesh& m, int slices)
{
    std::vector<float>    verts;
    std::vector<unsigned> idx;

    // Side wall (unit radius, Y: 0 → 1, outward normals)
    for (int i = 0; i <= slices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * i / slices;
        float cx = std::cos(theta), cz = std::sin(theta);
        verts.insert(verts.end(), { cx, 0.0f, cz,  cx, 0.0f, cz }); // bottom
        verts.insert(verts.end(), { cx, 1.0f, cz,  cx, 0.0f, cz }); // top
    }
    for (int i = 0; i < slices; ++i)
    {
        unsigned a = i * 2, b = a + 1, c = a + 2, d = a + 3;
        idx.insert(idx.end(), { a,b,c, b,d,c });
    }

    // Caps (normal pointing ±Y)
    auto addCap = [&](float y, float ny, bool flip)
        {
            unsigned center = (unsigned)(verts.size() / 6);
            verts.insert(verts.end(), { 0.0f, y, 0.0f,  0.0f, ny, 0.0f });
            unsigned rim = center + 1;
            for (int i = 0; i <= slices; ++i)
            {
                float theta = 2.0f * glm::pi<float>() * i / slices;
                verts.insert(verts.end(),
                    { std::cos(theta), y, std::sin(theta), 0.0f, ny, 0.0f });
            }
            for (int i = 0; i < slices; ++i)
            {
                if (!flip) idx.insert(idx.end(), { center, rim + i,   rim + i + 1 });
                else       idx.insert(idx.end(), { center, rim + i + 1, rim + i });
            }
        };
    addCap(0.0f, -1.0f, false); // bottom cap
    addCap(1.0f, 1.0f, true);  // top cap

    UploadMesh(m, verts, idx);
}

void OpenGLDC::BuildConeMesh(Mesh& m, int slices)
{
    std::vector<float>    verts;
    std::vector<unsigned> idx;

    // Lateral surface: apex at Y=1, base rim at Y=0, radius=1
    unsigned apex = 0;
    unsigned rimStart = 1;
    verts.insert(verts.end(), { 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f });

    for (int i = 0; i <= slices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * i / slices;
        float cx = std::cos(theta), cz = std::sin(theta);
        glm::vec3 n = glm::normalize(glm::vec3(cx, 0.5f, cz));
        verts.insert(verts.end(), { cx, 0.0f, cz,  n.x, n.y, n.z });
    }
    for (int i = 0; i < slices; ++i)
        idx.insert(idx.end(), { apex, rimStart + i + 1, rimStart + i });

    // Base cap (normal −Y)
    unsigned capCenter = (unsigned)(verts.size() / 6);
    verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f });
    unsigned capRim = capCenter + 1;
    for (int i = 0; i <= slices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * i / slices;
        verts.insert(verts.end(),
            { std::cos(theta), 0.0f, std::sin(theta),  0.0f, -1.0f, 0.0f });
    }
    for (int i = 0; i < slices; ++i)
        idx.insert(idx.end(), { capCenter, capRim + i, capRim + i + 1 });

    UploadMesh(m, verts, idx);
}

void OpenGLDC::FreeMesh(Mesh& m)
{
    if (m.ibo) { glDeleteBuffers(1, &m.ibo); m.ibo = 0; }
    if (m.vbo) { glDeleteBuffers(1, &m.vbo); m.vbo = 0; }
    if (m.vao) { glDeleteVertexArrays(1, &m.vao); m.vao = 0; }
    m.indexCount = 0;
}
 
bool OpenGLDC::CreateGLContext()
{

    unsigned pixelFormat;
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,              // Not concerned with these.
        0, 0, 0, 0, 0, 0, 0,           // No alpha or accum buffer.
        32,                            // 32-bit depth buffer.
        0, 0,                          // No stencil or aux buffer.
        PFD_MAIN_PLANE,                // Main layer type.
        0,                             // Reserved.
        0, 0, 0                        // Unsupported.
    };

    RECT    rt = { 0, 0, 0, 0 };
    GetClientRect(m_hWnd, &rt);
    m_hDC = GetDC(m_hWnd);

    ;

    if ((pixelFormat = ChoosePixelFormat(m_hDC, &pfd)) == 0)
    {
        printf("ChoosePixelFormat failed: %d\n", GetLastError());
        return  false;
    }

    if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
    {
        printf("SetPixelFormat failed: %d\n", GetLastError());
        return  false;
    }
    m_hRC = wglCreateContext(m_hDC);
    if (!wglMakeCurrent(m_hDC, m_hRC))
    {
        return  false;
    }

    gladLoadGL(); // 加载 OpenGL 函数指针

    return  true;
}

void OpenGLDC::DestroyGLContext()
{
    if (m_hRC)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hRC);
        m_hRC = nullptr;
    }
    if (m_hDC && m_hWnd)
    {
        ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }
}

// ============================================================================
// Shader helpers
// ============================================================================

GLuint OpenGLDC::CompileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        OutputDebugStringA("[GLSL compile error]\n");
        OutputDebugStringA(log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint OpenGLDC::LinkProgram(GLuint vs, GLuint fs)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024] = {};
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        OutputDebugStringA("[GLSL link error]\n");
        OutputDebugStringA(log);
        glDeleteProgram(prog);
        return 0;
    }
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    return prog;
}

bool OpenGLDC::BuildShaders()
{
    // ── Phong program ─────────────────────────────────────────────────────────
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS_Phong);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS_Phong);
        if (!vs || !fs) { glDeleteShader(vs); glDeleteShader(fs); return false; }

        m_progPhong = LinkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        if (!m_progPhong) return false;

        m_uMVP_phong = glGetUniformLocation(m_progPhong, "uMVP");
        m_uModel_phong = glGetUniformLocation(m_progPhong, "uModel");
        m_uNormalMat = glGetUniformLocation(m_progPhong, "uNormalMat");
        m_uMatColor = glGetUniformLocation(m_progPhong, "uMatColor");
        m_uLightDir = glGetUniformLocation(m_progPhong, "uLightDir");
        m_uLightAmbient = glGetUniformLocation(m_progPhong, "uLightAmbient");
        m_uLightDiffuse = glGetUniformLocation(m_progPhong, "uLightDiffuse");
        m_uShading = glGetUniformLocation(m_progPhong, "uShading");
    }

    // ── Flat (unlit) program ──────────────────────────────────────────────────
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS_Flat);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS_Flat);
        if (!vs || !fs) { glDeleteShader(vs); glDeleteShader(fs); return false; }

        m_progFlat = LinkProgram(vs, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
        if (!m_progFlat) return false;

        m_uMVP_flat = glGetUniformLocation(m_progFlat, "uMVP");
        m_uColor_flat = glGetUniformLocation(m_progFlat, "uColor");
    }

    return true;
}

void OpenGLDC::DeleteShaders()
{
    if (m_progPhong) { glDeleteProgram(m_progPhong); m_progPhong = 0; }
    if (m_progFlat) { glDeleteProgram(m_progFlat);  m_progFlat = 0; }
}