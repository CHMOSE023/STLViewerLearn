#include "Camera.h"

#include <glad/glad.h>    
#include <algorithm>
#include <cassert>
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

static constexpr double PI = 3.14159265358979323846;

Camera::Camera()
{
    Init();
}

void Camera::Init()
{
    // Y-up 坐标系：eye 在 Z 轴正方向看向原点，Y 轴朝上
    m_eye = glm::vec3(0.0f, 0.0f, 1000.0f);
    m_ref = glm::vec3(0.0f, 0.0f, 0.0f);
    m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f);  // Y is up

    m_far = 100000.0;
    m_near = 1.0;
    m_width = 2400.0;
    m_height = 2400.0;

    m_screenW = 400;
    m_screenH = 400;

    RebuildMatrices();
}

// ---------------------------------------------------------------------------
// Build GL matrices and set viewport – call once per frame before rendering
// ---------------------------------------------------------------------------
void Camera::Projection()
{
    glViewport(0, 0, m_screenW, m_screenH);
    RebuildMatrices();
}

void Camera::RebuildMatrices()
{
    float hw = static_cast<float>(m_width * 0.5);
    float hh = static_cast<float>(m_height * 0.5);

    m_projMatrix = glm::ortho(-hw, hw, -hh, hh,
        static_cast<float>(m_near),
        static_cast<float>(m_far));

    m_viewMatrix = glm::lookAt(m_eye, m_ref, m_vecUp);
}

// ---------------------------------------------------------------------------
// Build a world-space pick ray from a screen-space pixel (replaces old
// gluPickMatrix / GL_SELECT approach – use ray-object intersection instead)
// ---------------------------------------------------------------------------
void Camera::GetPickRay(int xPos, int yPos,
    glm::vec3& rayOrigin, glm::vec3& rayDir) const
{
    // NDC: map pixel to [-1, 1]
    float ndcX = (2.0f * xPos) / m_screenW - 1.0f;
    float ndcY = -(2.0f * yPos) / m_screenH + 1.0f;   // flip Y

    glm::mat4 invVP = glm::inverse(m_projMatrix * m_viewMatrix);

    glm::vec4 nearPt = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 farPt = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

    nearPt /= nearPt.w;
    farPt /= farPt.w;

    rayOrigin = glm::vec3(nearPt);
    rayDir = glm::normalize(glm::vec3(farPt) - glm::vec3(nearPt));
}

// ---------------------------------------------------------------------------
// Viewport resize
// ---------------------------------------------------------------------------
void Camera::SetScreen(int w, int h)
{
    if (h == 0) h = 1;
    if (w == 0) w = 1;

    double ratioNew = static_cast<double>(w) / h;
    double ratioOld = static_cast<double>(m_screenW) / m_screenH;

    m_width *= static_cast<double>(w) / m_screenW;
    m_height *= static_cast<double>(h) / m_screenH;
    m_width = m_height * ratioNew;   // keep aspect

    m_screenW = w;
    m_screenH = h;

    glViewport(0, 0, w, h);
}

// ---------------------------------------------------------------------------
// Zoom
// ---------------------------------------------------------------------------
void Camera::Zoom(double scale)
{
    assert(scale > 0.0);
    m_width *= scale;
    m_height *= scale;
}

void Camera::ZoomAll(double x0, double y0, double z0,
    double x1, double y1, double z1)
{
    double xl = x1 - x0;
    double yl = y1 - y0;
    double zl = z1 - z0;
    double sz = std::max({ xl, yl, zl });

    SetViewRect(sz, sz);

    glm::vec3 vec = m_eye - m_ref;
    m_ref = glm::vec3(
        static_cast<float>((x0 + x1) * 0.5),
        static_cast<float>((y0 + y1) * 0.5),
        static_cast<float>((z0 + z1) * 0.5));
    m_eye = m_ref + vec;
}

// ---------------------------------------------------------------------------
// Pan
// ---------------------------------------------------------------------------
void Camera::MoveScreen(int dx, int dy)
{
    double dxvw = static_cast<double>(dx) * m_width / m_screenW;
    double dyvw = static_cast<double>(dy) * m_height / m_screenH;

    glm::vec3 forward = glm::normalize(m_ref - m_eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_vecUp));
    glm::vec3 up = glm::cross(right, forward);

    glm::vec3 delta = right * static_cast<float>(dxvw)
        - up * static_cast<float>(dyvw);

    m_eye -= delta;
    m_ref -= delta;
}

// ---------------------------------------------------------------------------
// Mouse helpers
// ---------------------------------------------------------------------------
void Camera::OnOrbit(int dx, int dy)
{
    // dx > 0 (mouse right) → scene rotates right → camera turns right
    // dy > 0 (mouse down)  → scene rotates down  → camera turns down
    TurnRight(dx * kOrbitSpeed);
    TurnDown(dy * kOrbitSpeed);
}

void Camera::OnPan(int dx, int dy)
{
    MoveScreen(dx, dy);
}

void Camera::OnZoom(int delta)
{
    double scale = (delta > 0) ? (1.0 - kZoomSpeed) : (1.0 + kZoomSpeed);
    Zoom(scale);
}

// ---------------------------------------------------------------------------
// Orbit – rotate eye around ref
// ---------------------------------------------------------------------------
static glm::mat4 RotateAround(const glm::vec3& axis, double angleDeg)
{
    return glm::rotate(glm::mat4(1.0f),
        static_cast<float>(angleDeg * PI / 180.0),
        axis);
}

// ---------------------------------------------------------------------------
// 球坐标辅助 - Three.js OrbitControls 同款方案
// phi   = 从 +Y 轴向下的极角  (0 = 正上方, π/2 = 水平, π = 正下方)
// theta = 绕 Y 轴的方位角
// 优点：完全不依赖 cross product，没有极点退化问题
// ---------------------------------------------------------------------------

struct Spherical {
    float r = 1.0f;
    float phi = glm::pi<float>() / 2.0f;
    float theta = 0.0f;
};

static Spherical ToSpherical(const glm::vec3& v)
{
    Spherical s;
    s.r = glm::length(v);
    if (s.r < 1e-6f) return s;
    s.phi = std::acos(glm::clamp(v.y / s.r, -1.0f, 1.0f));
    s.theta = std::atan2(v.x, v.z);
    return s;
}

static glm::vec3 FromSpherical(const Spherical& s)
{
    float sinPhi = std::sin(s.phi);
    return glm::vec3(
        s.r * sinPhi * std::sin(s.theta),  // x
        s.r * std::cos(s.phi),             // y
        s.r * sinPhi * std::cos(s.theta)   // z
    );
}

void Camera::UpdateUpVec()
{
    // 球坐标方案下，phi 已被限制在 (1°, 179°)，eye 永远不会真正到达极点
    // vecUp 始终给世界 Y 轴即可，glm::lookAt 能正确处理
    m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::TurnLeft(double angle)
{
    glm::vec3 vec = m_eye - m_ref;
    Spherical s = ToSpherical(vec);
    s.theta += static_cast<float>(angle * PI / 180.0);
    m_eye = m_ref + FromSpherical(s);
    UpdateUpVec();
}

void Camera::TurnRight(double angle)
{
    TurnLeft(-angle);
}

void Camera::TurnUp(double angle)
{
    glm::vec3 vec = m_eye - m_ref;
    Spherical s = ToSpherical(vec);

    // phi 减小 = 向上仰，限制在 (1°, 179°) 防止穿越极点
    static constexpr float kMinPhi = glm::pi<float>() * 1.0f / 180.0f;
    static constexpr float kMaxPhi = glm::pi<float>() * 179.0f / 180.0f;

    s.phi += static_cast<float>(angle * PI / 180.0);
    s.phi = glm::clamp(s.phi, kMinPhi, kMaxPhi);

    m_eye = m_ref + FromSpherical(s);
    UpdateUpVec();
}

void Camera::TurnDown(double angle)
{
    TurnUp(-angle);
}

// ---------------------------------------------------------------------------
// View presets
// ---------------------------------------------------------------------------
void Camera::SetViewType(ViewType type)
{
    glm::vec3 vec = m_eye - m_ref;
    float r = glm::length(vec);
    if (r < 1e-6f) r = 50.0f;
    if (r > 10000.f) r = 10000.f;

    switch (type)
    {
        // Z-up CAD convention:
        //   Front  = looking from -Y toward origin, up = +Z
        //   Back   = looking from +Y toward origin, up = +Z
        //   Top    = looking from +Z toward origin, up = +Y
        //   Bottom = looking from -Z toward origin, up = +Y
        //   Right  = looking from +X toward origin, up = +Z
        //   Left   = looking from -X toward origin, up = +Z
    case ViewType::Front:
        m_eye = m_ref + glm::vec3(0.0f, -r, 0.0f);
        m_vecUp = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case ViewType::Back:
        m_eye = m_ref + glm::vec3(0.0f, r, 0.0f);
        m_vecUp = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case ViewType::Top:
        m_eye = m_ref + glm::vec3(0.0f, 0.0f, r);
        m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f);  // +Y forward when looking down Z
        break;
    case ViewType::Bottom:
        m_eye = m_ref + glm::vec3(0.0f, 0.0f, -r);
        m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f);
        break;
    case ViewType::Right:
        m_eye = m_ref + glm::vec3(r, 0.0f, 0.0f);
        m_vecUp = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case ViewType::Left:
        m_eye = m_ref + glm::vec3(-r, 0.0f, 0.0f);
        m_vecUp = glm::vec3(0.0f, 0.0f, 1.0f);
        break;
    case ViewType::SW_Isometric:
        m_eye = m_ref + glm::normalize(glm::vec3(-1, -1, 1)) * r;
        UpdateUpVec(); break;
    case ViewType::SE_Isometric:
        m_eye = m_ref + glm::normalize(glm::vec3(1, -1, 1)) * r;
        UpdateUpVec(); break;
    case ViewType::NE_Isometric:
        m_eye = m_ref + glm::normalize(glm::vec3(1, 1, 1)) * r;
        UpdateUpVec(); break;
    case ViewType::NW_Isometric:
        m_eye = m_ref + glm::normalize(glm::vec3(-1, 1, 1)) * r;
        UpdateUpVec(); break;
    }
}

// ---------------------------------------------------------------------------
// View rect
// ---------------------------------------------------------------------------
void Camera::SetViewRect(double w, double h)
{
    m_height = h;
    double aspect = static_cast<double>(m_screenW) / m_screenH;
    m_width = m_height * aspect;
    (void)w;
}

void Camera::GetViewRect(double& w, double& h) const
{
    w = m_width;
    h = m_height;
}