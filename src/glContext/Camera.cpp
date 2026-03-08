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
    m_eye = glm::vec3(0.0f, 0.0f, 1000.0f); // eye on Z axis (front view)
    m_ref = glm::vec3(0.0f, 0.0f, 0.0f);
    m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f); // Y is up

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

void Camera::UpdateUpVec()
{
    // Y-up world coordinate system
    glm::vec3 forward = glm::normalize(m_ref - m_eye);
    glm::vec3 yWorld = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, yWorld));
    m_vecUp = glm::normalize(glm::cross(right, forward));
}

void Camera::TurnLeft(double angle)
{
    glm::mat4 rot = RotateAround(m_vecUp, angle);
    glm::vec3 vec = m_eye - m_ref;
    vec = glm::vec3(rot * glm::vec4(vec, 0.0f));
    m_eye = m_ref + vec;
    UpdateUpVec();
}

void Camera::TurnRight(double angle)
{
    TurnLeft(-angle);
}

void Camera::TurnUp(double angle)
{
    // Y-up: elevation = arcsin( (eye-ref).y / length )
    // positive Y = looking up, negative Y = looking down
    glm::vec3 vec = m_eye - m_ref;
    float     len = glm::length(vec);
    float     elevNow = glm::degrees(std::asin(
        glm::clamp(vec.y / len, -1.0f, 1.0f)));

    // Clamp elevation to (-89°, +89°) to prevent gimbal flip past Y poles
    static constexpr float kMaxElev = 89.0f;
    static constexpr float kMinElev = -89.0f;

    float elevNew = elevNow + static_cast<float>(angle);
    if (elevNew > kMaxElev) angle = kMaxElev - elevNow;
    if (elevNew < kMinElev) angle = kMinElev - elevNow;

    if (std::abs(angle) < 1e-5) return;

    glm::vec3 forward = glm::normalize(m_ref - m_eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_vecUp));
    glm::mat4 rot = RotateAround(right, angle);
    vec = glm::vec3(rot * glm::vec4(vec, 0.0f));
    m_eye = m_ref + vec;
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
        m_vecUp = glm::vec3(0.0f, 1.0f, 0.0f);
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