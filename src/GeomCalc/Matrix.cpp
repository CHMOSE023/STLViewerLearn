#include "Matrix.h"
#include "Vector.h"
#include <cmath>

// ============================================================
//  Matrix.cpp
//  矩阵运算实现
//
//  【行向量约定下的矩阵布局】
//    所有工厂函数写的矩阵，都是"列向量标准矩阵的转置"。
//    原因：列向量约定 → v' = M * v（列向量在右）
//          行向量约定 → v' = v * Mᵀ（行向量在左）
//    因此本库所有矩阵 = 教科书矩阵的转置。
// ============================================================

// ── Matrix2D ─────────────────────────────────────────────────

// 设为单位矩阵（对角线全 1，其余为 0）
void Matrix2D::Identity() {
    A = { {{1,0,0},{0,1,0},{0,0,1}} };
}

// 矩阵乘法：标准行×列方式（O(n³)，n=3）
// 用于组合变换：M_combined = M1 * M2 等价于先应用 M1 再应用 M2
Matrix2D Matrix2D::operator*(const Matrix2D& m) const {
    Matrix2D result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            result.A[i][j] = 0;
            for (int k = 0; k < 3; ++k)
                result.A[i][j] += A[i][k] * m.A[k][j];
        }
    return result;
}

Matrix2D& Matrix2D::operator*=(const Matrix2D& m) { return *this = *this * m; }

// 行列式（3×3 Sarrus 法则展开）
double Matrix2D::Determinant() const {
    return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
        - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
        + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
}

Matrix2D Matrix2D::MakeIdentity() {
    return Matrix2D{};
}

// ── 2D 旋转矩阵 ───────────────────────────────────────────────
// 列向量标准逆时针旋转：
//   |  cosθ  -sinθ |
//   |  sinθ   cosθ |
//
// 行向量约定取转置：
//   |  cosθ   sinθ |   → 存为 A[0]={ c, s, 0}
//   | -sinθ   cosθ |            A[1]={-s, c, 0}
//                               A[2]={ 0, 0, 1}
Matrix2D Matrix2D::MakeRotation(double angle) {
    Matrix2D m;
    double c = std::cos(angle), s = std::sin(angle);
    m.A = { {{c, s, 0},{-s, c, 0},{0, 0, 1}} };
    return m;
}

// ── 2D 缩放矩阵 ───────────────────────────────────────────────
// 均匀缩放，X 和 Y 方向等比例放大/缩小 s 倍
Matrix2D Matrix2D::MakeScale(double s) {
    Matrix2D m;
    m.A = { {{s,0,0},{0,s,0},{0,0,1}} };
    return m;
}

// ── 2D 平移矩阵 ───────────────────────────────────────────────
// 行向量约定：平移量存放在最后一行（第 2 行，索引从 0 起）
// [x, y, 1] × M = [x + tx, y + ty, 1]
//   A[2][0] = tx, A[2][1] = ty
Matrix2D Matrix2D::MakeTranslation(Vector2D v) {
    Matrix2D m;
    m.A = { {{1,0,0},{0,1,0},{v.dx,v.dy,1}} };
    return m;
}

// ── 2D 镜像矩阵 ───────────────────────────────────────────────
// 沿过原点、方向为 axis 的直线做镜像
// 设单位方向向量 n = (nx, ny)，镜像公式：
//   v' = 2(v·n)n - v
// 展开为矩阵（列向量式）：
//   | 1-2ny²   2nx*ny |
//   | 2nx*ny   1-2nx² |
// 行向量约定取转置（此矩阵为对称矩阵，转置等于自身）
Matrix2D Matrix2D::MakeMirror(Vector2D axis) {
    auto n = axis.Normalized();
    double nx = n.dx, ny = n.dy;
    Matrix2D m;
    m.A = { {{1 - 2 * ny * ny, 2 * nx * ny, 0},
             {2 * nx * ny, 1 - 2 * nx * nx, 0},
             {0,           0,               1}} };
    return m;
}

// ── Matrix3D ─────────────────────────────────────────────────

void Matrix3D::Identity() {
    A = { {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}} };
}

// 矩阵乘法（O(n³)，n=4）
Matrix3D Matrix3D::operator*(const Matrix3D& m) const {
    Matrix3D result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            result.A[i][j] = 0;
            for (int k = 0; k < 4; ++k)
                result.A[i][j] += A[i][k] * m.A[k][j];
        }
    return result;
}

Matrix3D& Matrix3D::operator*=(const Matrix3D& m) { return *this = *this * m; }

Matrix3D Matrix3D::MakeIdentity() {
    return Matrix3D{};
}

// ── 3D 旋转矩阵（Rodrigues 公式）────────────────────────────
// 绕过原点的任意轴 axis 旋转 angle 弧度（右手定则逆时针）
//
// 设单位轴向量 n = (x, y, z)，c = cosθ，s = sinθ，t = 1-cosθ
//
// 列向量标准 Rodrigues 旋转矩阵：
//   | t*x*x+c    t*x*y-s*z  t*x*z+s*y |
//   | t*x*y+s*z  t*y*y+c    t*y*z-s*x |
//   | t*x*z-s*y  t*y*z+s*x  t*z*z+c   |
//
// 行向量约定取转置（交换 [i][j] 和 [j][i]，即 ±s*? 项符号对调）：
//   A[0] = { t*x*x+c,    t*x*y+s*z,  t*x*z-s*y }
//   A[1] = { t*x*y-s*z,  t*y*y+c,    t*y*z+s*x }
//   A[2] = { t*x*z+s*y,  t*y*z-s*x,  t*z*z+c   }
Matrix3D Matrix3D::MakeRotation(double angle, Vector3D axis) {
    auto n = axis.Normalized();
    double c = std::cos(angle), s = std::sin(angle), t = 1 - c;
    double x = n.dx, y = n.dy, z = n.dz;
    Matrix3D m;
    m.A = { {
        {t * x * x + c,     t * x * y + s * z,  t * x * z - s * y,  0},
        {t * x * y - s * z,   t * y * y + c,    t * y * z + s * x,  0},
        {t * x * z + s * y,   t * y * z - s * x,  t * z * z + c,    0},
        {0,           0,          0,           1}
    } };
    return m;
}

// ── 3D 缩放矩阵 ───────────────────────────────────────────────
Matrix3D Matrix3D::MakeScale(double s) {
    Matrix3D m;
    m.A = { {{s,0,0,0},{0,s,0,0},{0,0,s,0},{0,0,0,1}} };
    return m;
}

// ── 3D 平移矩阵 ───────────────────────────────────────────────
// 行向量约定：平移量存放在第四行（索引 3）
// [x, y, z, 1] × M → x 分量 = x*1 + ... + 1*tx
Matrix3D Matrix3D::MakeTranslation(Vector3D v) {
    Matrix3D m;
    m.A = { {{1,0,0,0},{0,1,0,0},{0,0,1,0},{v.dx,v.dy,v.dz,1}} };
    return m;
}

// ── 3D 镜像矩阵 ───────────────────────────────────────────────
// 沿过原点、法向量为 planeNormal 的平面做镜像
// 设单位法向量 n = (x, y, z)，镜像公式：
//   v' = v - 2(v·n)n
// 展开为矩阵（对称矩阵，转置等于自身）：
//   | 1-2x²   -2xy    -2xz   |
//   | -2xy    1-2y²   -2yz   |
//   | -2xz    -2yz    1-2z²  |
Matrix3D Matrix3D::MakeMirror(Vector3D planeNormal) {
    auto n = planeNormal.Normalized();
    double x = n.dx, y = n.dy, z = n.dz;
    Matrix3D m;
    m.A = { {
        {1 - 2 * x * x,  -2 * x * y,   -2 * x * z,   0},
        {-2 * x * y,   1 - 2 * y * y,  -2 * y * z,   0},
        {-2 * x * z,   -2 * y * z,   1 - 2 * z * z,  0},
        {0,        0,        0,         1}
    } };
    return m;
}

// ── 行列式（取左上 3×3 子矩阵）──────────────────────────────
// 按第一行展开（代数余子式展开法）
// 用途：判断变换是否包含镜像（det < 0）或矩阵是否可逆（det ≠ 0）
double Matrix3D::Determinant() const {
    double det = 0;
    for (int j = 0; j < 3; ++j) {
        int j1 = (j + 1) % 3, j2 = (j + 2) % 3;
        det += A[0][j] * (A[1][j1] * A[2][j2] - A[1][j2] * A[2][j1]);
    }
    return det;
}


// ── Matrix4D ─────────────────────────────────────────────────

void Matrix4D::Identity()
{
    A = { {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1}
    } };
}

// 矩阵乘法
Matrix4D Matrix4D::operator*(const Matrix4D& m) const
{
    Matrix4D result;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
        {
            result.A[i][j] = 0;

            for (int k = 0; k < 4; ++k)
                result.A[i][j] += A[i][k] * m.A[k][j];
        }

    return result;
}

Matrix4D& Matrix4D::operator*=(const Matrix4D& m)
{
    return *this = *this * m;
}

// 行列式（取左上 3×3）
double Matrix4D::Determinant() const
{
    double det = 0;

    for (int j = 0; j < 3; ++j)
    {
        int j1 = (j + 1) % 3;
        int j2 = (j + 2) % 3;

        det += A[0][j] *
            (A[1][j1] * A[2][j2] - A[1][j2] * A[2][j1]);
    }

    return det;
}

Matrix4D Matrix4D::MakeIdentity()
{
    return Matrix4D{};
}


Matrix4D Matrix4D::MakeRotation(double angle, Vector3D axis)
{
    auto n = axis.Normalized();

    double x = n.dx;
    double y = n.dy;
    double z = n.dz;

    double c = std::cos(angle);
    double s = std::sin(angle);
    double t = 1 - c;

    Matrix4D m;

    m.A = { {
        {t * x * x + c,     t * x * y + s * z,   t * x * z - s * y,   0},
        {t * x * y - s * z,   t * y * y + c,     t * y * z + s * x,   0},
        {t * x * z + s * y,   t * y * z - s * x,   t * z * z + c,     0},
        {0,             0,             0,             1}
    } };

    return m;
}


Matrix4D Matrix4D::MakeScale(double s)
{
    Matrix4D m;

    m.A = { {
        {s,0,0,0},
        {0,s,0,0},
        {0,0,s,0},
        {0,0,0,1}
    } };

    return m;
}


Matrix4D Matrix4D::MakeTranslation(Vector3D v)
{
    Matrix4D m;

    m.A = { {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {v.dx, v.dy, v.dz, 1}
    } };

    return m;
}


Matrix4D Matrix4D::MakeMirror(Vector3D planeNormal)
{
    auto n = planeNormal.Normalized();

    double x = n.dx;
    double y = n.dy;
    double z = n.dz;

    Matrix4D m;

    m.A = { {
        {1 - 2 * x * x,  -2 * x * y,   -2 * x * z,   0},
        {-2 * x * y,   1 - 2 * y * y,  -2 * y * z,   0},
        {-2 * x * z,   -2 * y * z,   1 - 2 * z * z,  0},
        {0,        0,        0,         1}
    } };

    return m;
}

void  Matrix4D::ToFloatArray( float f[16])  const
{
    int index = 0;

    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            f[index++] = static_cast<float>(A[r][c]);
}
 
 