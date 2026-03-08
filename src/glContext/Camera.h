#pragma once

#include <glm/glm.hpp>


// View type constants
enum class ViewType
{
    Front = 0,
    Back = 1,
    Top = 2,
    Bottom = 3,
    Right = 4,
    Left = 5,
    SW_Isometric = 6,
    SE_Isometric = 7,
    NE_Isometric = 8,
    NW_Isometric = 9,
};

class Camera
{
public:
    Camera();
    ~Camera() = default;

    // Initialization
    void Init();

    // Projection: fills m_projMatrix and m_viewMatrix, calls glViewport
    void Projection();

    // Selection picking mode (replaces glRenderMode/gluPickMatrix pattern)
    // Returns pick-ray origin and direction in world space
    void GetPickRay(int xPos, int yPos,
        glm::vec3& rayOrigin, glm::vec3& rayDir) const;

    // Viewport resize
    void SetScreen(int w, int h);

    // View control
    void SetViewType(ViewType type);

    // Zoom: scale < 1 zooms in, scale > 1 zooms out
    void Zoom(double scale);
    void ZoomAll(double x0, double y0, double z0,
        double x1, double y1, double z1);

    // Pan in screen pixels
    void MoveScreen(int dx, int dy);

    // Orbit (degrees)
    void TurnLeft(double angle);
    void TurnRight(double angle);
    void TurnUp(double angle);
    void TurnDown(double angle);

    // Mouse drag helpers (called with delta pixels)
    void OnOrbit(int dx, int dy);   // left-drag  → orbit
    void OnPan(int dx, int dy);   // mid-drag   → pan
    void OnZoom(int delta);        // wheel      → zoom

    // Accessors
    const glm::mat4& GetViewMatrix()       const { return m_viewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_projMatrix; }
    glm::mat4        GetViewProjMatrix()   const { return m_projMatrix * m_viewMatrix; }

    void SetEye(const glm::vec3& eye) { m_eye = eye; }
    void SetRef(const glm::vec3& ref) { m_ref = ref; }
    void SetUpVec(const glm::vec3& up) { m_vecUp = up; }

    glm::vec3 GetEye()   const { return m_eye; }
    glm::vec3 GetRef()   const { return m_ref; }
    glm::vec3 GetUpVec() const { return m_vecUp; }

    void   SetViewRect(double w, double h);
    void   GetViewRect(double& w, double& h) const;

    int    GetScreenW() const { return m_screenW; }
    int    GetScreenH() const { return m_screenH; }

private:
    void RebuildMatrices();
    void UpdateUpVec();

    // Camera state
    glm::vec3 m_eye;
    glm::vec3 m_ref;
    glm::vec3 m_vecUp;

    // Orthographic frustum half-sizes
    double m_width = 2400.0;
    double m_height = 2400.0;
    double m_near = 1.0;
    double m_far = 100000.0;

    // Viewport
    int m_screenW = 400;
    int m_screenH = 400;

    // Cached matrices (rebuilt every frame in Projection())
    glm::mat4 m_viewMatrix = glm::mat4(1.0f);
    glm::mat4 m_projMatrix = glm::mat4(1.0f);

    // Orbit sensitivity (degrees per pixel)
    static constexpr double kOrbitSpeed = 0.3;
    static constexpr double kZoomSpeed = 0.1;
};