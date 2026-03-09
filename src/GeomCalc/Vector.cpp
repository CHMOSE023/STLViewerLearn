#include "Vector.h"
#include "Matrix.h"
#include <cmath>
#include <stdexcept>

// ============================================================
//  Vector.cpp
//  向量与矩阵相关实现
//
//  【行向量乘法原理】
//    行向量 × 矩阵：result[j] = Σ v[i] * M[i][j]
//    向量变换只取矩阵左上角的旋转/缩放部分，
//    忽略最后一行的平移（平移对自由向量无意义）。
// ============================================================

// ── Vector2D × Matrix2D ──────────────────────────────────────
// 2D 向量只参与旋转和缩放（2×2 子矩阵），不受平移影响。
// 计算：result.dx = dx*M[0][0] + dy*M[1][0]
//       result.dy = dx*M[0][1] + dy*M[1][1]
Vector2D Vector2D::operator*(const Matrix2D& m) const {
    return {
        dx * m.A[0][0] + dy * m.A[1][0],
        dx * m.A[0][1] + dy * m.A[1][1]
    };
}
Vector2D& Vector2D::operator*=(const Matrix2D& m) { return *this = *this * m; }

// ── Vector3D × Matrix3D ──────────────────────────────────────
// 3D 向量只参与旋转和缩放（3×3 子矩阵），不受平移影响。
// 计算：result.dx = dx*M[0][0] + dy*M[1][0] + dz*M[2][0]
//       result.dy = dx*M[0][1] + dy*M[1][1] + dz*M[2][1]
//       result.dz = dx*M[0][2] + dy*M[1][2] + dz*M[2][2]
Vector3D Vector3D::operator*(const Matrix3D& m) const {
    return {
        dx * m.A[0][0] + dy * m.A[1][0] + dz * m.A[2][0],
        dx * m.A[0][1] + dy * m.A[1][1] + dz * m.A[2][1],
        dx * m.A[0][2] + dy * m.A[1][2] + dz * m.A[2][2]
    };
}
Vector3D& Vector3D::operator*=(const Matrix3D& m) { return *this = *this * m; }
 