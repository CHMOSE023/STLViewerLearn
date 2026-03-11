#pragma once
#include "CadBase.h"    
#include "Box.h"    
#include "Point.h"    
#include "Vector.h"    
#include "Matrix.h"    

#ifdef GLM_VERSION_MAJOR
#include <glm/glm.hpp>  

inline glm::vec3  ToGLM(const Vector3D& v) { return { (float)v.dx,(float)v.dy,(float)v.dz }; }
inline glm::vec3  ToGLM(const Point3D& p) { return { (float)p.x,(float)p.y,(float)p.z }; }
inline glm::dvec3 ToGLMd(const Vector3D& v) { return { v.dx, v.dy, v.dz }; }
inline glm::dvec3 ToGLMd(const Point3D& p) { return { p.x, p.y, p.z }; }

inline Vector3D   FromGLM(const glm::vec3& v) { return { v.x, v.y, v.z }; }
inline Point3D FromGLMP(const glm::vec3& v) { return { v.x, v.y, v.z }; }

inline glm::mat4 ToGLM(const Matrix4D& m)
{
    float f[16]; m.ToFloatArray(f);
    return glm::make_mat4(f);
}
// namespace GeomCalc
#endif // GLM_VERSION_MAJOR
