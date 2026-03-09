#include "Point.h"
#include "Vector.h"
#include "Matrix.h"
#include <cmath>

// ============================================================
//  Point.cpp
//
//  【矩阵乘法的行向量展开】
//
//  2D 点使用 3×3 齐次矩阵，点表示为行向量 [x, y, 1]：
//
//                  | M[0][0]  M[0][1]  0 |
//    [x, y, 1]  ×  | M[1][0]  M[1][1]  0 |
//                  | M[2][0]  M[2][1]  1 |
//
//    result.x = x*M[0][0] + y*M[1][0] + 1*M[2][0]
//    result.y = x*M[0][1] + y*M[1][1] + 1*M[2][1]
//    （第三分量恒为 1，不需要存储）
//
//  3D 点使用 4×4 齐次矩阵，点表示为行向量 [x, y, z, 1]：
//    result.x = x*M[0][0] + y*M[1][0] + z*M[2][0] + 1*M[3][0]
//    result.y = x*M[0][1] + y*M[1][1] + z*M[2][1] + 1*M[3][1]
//    result.z = x*M[0][2] + y*M[1][2] + z*M[2][2] + 1*M[3][2]
// ============================================================

// ── Point2D ───────────────────────────────────────────────────

Point2D Point2D::operator+(const Vector2D& v) const { return { x + v.dx, y + v.dy }; }
Point2D Point2D::operator-(const Vector2D& v) const { return { x - v.dx, y - v.dy }; }

Point2D& Point2D::operator+=(const Vector2D& v) { x += v.dx; y += v.dy; return *this; }
Point2D& Point2D::operator-=(const Vector2D& v) { x -= v.dx; y -= v.dy; return *this; }

// 两点相减：返回从 p 指向 *this 的向量
Vector2D Point2D::operator-(const Point2D& p) const { return { x - p.x, y - p.y }; }

// 2D 点变换：行向量 [x, y, 1] × 3×3 矩阵
Point2D Point2D::operator*(const Matrix2D& m) const {
    return {
        x * m.A[0][0] + y * m.A[1][0] + m.A[2][0],   // 含平移 M[2][0]
        x * m.A[0][1] + y * m.A[1][1] + m.A[2][1]    // 含平移 M[2][1]
    };
}
Point2D& Point2D::operator*=(const Matrix2D& m) { return *this = *this * m; }

// ── Point3D ───────────────────────────────────────────────────

Point3D Point3D::operator+(const Vector3D& v) const { return { x + v.dx, y + v.dy, z + v.dz }; }
Point3D Point3D::operator-(const Vector3D& v) const { return { x - v.dx, y - v.dy, z - v.dz }; }

Point3D& Point3D::operator+=(const Vector3D& v) { x += v.dx; y += v.dy; z += v.dz; return *this; }
Point3D& Point3D::operator-=(const Vector3D& v) { x -= v.dx; y -= v.dy; z -= v.dz; return *this; }

// 两点相减：返回从 p 指向 *this 的向量
Vector3D Point3D::operator-(const Point3D& p) const { return { x - p.x, y - p.y, z - p.z }; }

// 3D 点变换：行向量 [x, y, z, 1] × 4×4 矩阵
Point3D Point3D::operator*(const Matrix3D& m) const {
    return {
        x * m.A[0][0] + y * m.A[1][0] + z * m.A[2][0] + m.A[3][0],   // 含平移 M[3][0]
        x * m.A[0][1] + y * m.A[1][1] + z * m.A[2][1] + m.A[3][1],   // 含平移 M[3][1]
        x * m.A[0][2] + y * m.A[1][2] + z * m.A[2][2] + m.A[3][2]    // 含平移 M[3][2]
    };
}
Point3D& Point3D::operator*=(const Matrix3D& m) { return *this = *this * m; }

