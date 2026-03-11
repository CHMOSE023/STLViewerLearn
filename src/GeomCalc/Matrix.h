#pragma once
#include <array>
#include "CadBase.h"

// ============================================================
//  Matrix.h
//  二维变换矩阵（Matrix2D）与三维变换矩阵（Matrix3D）
//
//  【齐次坐标与矩阵】
//    为了用一个矩阵同时表达旋转、缩放和平移，
//    引入"齐次坐标"（Homogeneous Coordinates）：
//      2D 点 (x, y)    → 行向量 [x, y, 1]，使用 3×3 矩阵
//      3D 点 (x, y, z) → 行向量 [x, y, z, 1]，使用 4×4 矩阵
//
//  【行向量约定（Row-Vector Convention）】
//    本库统一使用"行向量 × 矩阵"顺序：
//      变换后 = 原始点 * 矩阵
//    与 OpenGL（列向量，矩阵 × 列向量）相反。
//    效果：矩阵的平移部分存放在最后一"行"（而非最后一"列"）。
//
//  【变换组合顺序】
//    先平移再旋转：M = 平移矩阵 * 旋转矩阵
//    先旋转再平移：M = 旋转矩阵 * 平移矩阵
//    注意：与列向量约定的顺序相反。
// ============================================================

// ============================================================
//  Matrix2D —— 二维齐次变换矩阵（3×3）
//
//  矩阵布局（行向量约定）：
//    | r00  r01  0 |   r: 旋转/缩放部分
//    | r10  r11  0 |
//    | tx   ty   1 |   tx, ty: X/Y 方向平移量
// ============================================================
class Matrix2D {
public:
    // 使用 std::array 代替裸数组，支持拷贝、比较，更安全
    std::array<std::array<double, 3>, 3> A{};

    // 默认构造：初始化为单位矩阵（Identity Matrix）
    Matrix2D() { Identity(); }

    // 从二维数组直接构造（用于矩阵字面量初始化）
    explicit Matrix2D(const std::array<std::array<double, 3>, 3>& data) : A(data) {}

    // ── 矩阵乘法 ──────────────────────────────────────────────
    // 用于组合多个变换，结果等价于依次应用各变换
    Matrix2D  operator*(const Matrix2D& m) const;
    Matrix2D& operator*=(const Matrix2D& m);

    // ── 设为单位矩阵 ──────────────────────────────────────────
    // 单位矩阵：对任意点/向量乘以它，结果不变
    void Identity();

    // ── 行列式 ────────────────────────────────────────────────
    // det(M) 非零：矩阵可逆（变换可撤销）
    // det(M) < 0 ：变换包含镜像（翻转了坐标系方向）
    double Determinant() const;

    // ── 工厂函数（Factory Functions）────────────────────────────
    // 统一使用 make* 命名，返回对应变换矩阵

    // 单位矩阵（无变换）
    static Matrix2D MakeIdentity();

    // 镜像矩阵：沿过原点、方向为 axis 的直线做镜像
    static Matrix2D MakeMirror(Vector2D axis);

    // 旋转矩阵：逆时针旋转 angleRad 弧度
    // 行向量约定下，矩阵为列向量标准旋转矩阵的转置
    static Matrix2D MakeRotation(double angleRad);

    // 均匀缩放矩阵：以原点为中心缩放 s 倍
    static Matrix2D MakeScale(double s);

    // 平移矩阵：沿向量 v 方向平移
    // 行向量约定下，平移量存放在矩阵第三行
    static Matrix2D MakeTranslation(Vector2D v);
};

// ============================================================
//  Matrix3D —— 三维齐次变换矩阵（4×4）
//
//  矩阵布局（行向量约定）：
//    | r00  r01  r02  0 |   r: 旋转/缩放部分（3×3 子矩阵）
//    | r10  r11  r12  0 |
//    | r20  r21  r22  0 |
//    | tx   ty   tz   1 |   tx, ty, tz: 平移量
// ============================================================
class Matrix3D {
public:
    std::array<std::array<double, 4>, 4> A{};

    // 默认构造：初始化为单位矩阵
    Matrix3D() { Identity(); }

    explicit Matrix3D(const std::array<std::array<double, 4>, 4>& data) : A(data) {}

    // ── 矩阵乘法 ──────────────────────────────────────────────
    Matrix3D  operator*(const Matrix3D& m) const;
    Matrix3D& operator*=(const Matrix3D& m);

    // ── 设为单位矩阵 ──────────────────────────────────────────
    void Identity();

    // ── 行列式（取左上 3×3 子矩阵）──────────────────────────
    double Determinant() const;

    // ── 工厂函数 ──────────────────────────────────────────────

    // 单位矩阵
    static Matrix3D MakeIdentity();

    // 镜像矩阵：沿过原点、法向量为 planeNormal 的平面做镜像
    static Matrix3D MakeMirror(Vector3D planeNormal);

    // 旋转矩阵：绕过原点的 axis 轴，逆时针旋转 angleRad 弧度
    // 使用 Rodrigues 旋转公式；行向量约定下取标准矩阵的转置
    static Matrix3D MakeRotation(double angleRad, Vector3D axis);

    // 均匀缩放矩阵
    static Matrix3D MakeScale(double s);

    // 平移矩阵：沿向量 v 方向平移
    // 行向量约定下，平移量存放在矩阵第四行
    static Matrix3D MakeTranslation(Vector3D v);
};


class Matrix4D {
public:
    std::array<std::array<double, 4>, 4> A{};
    // 默认构造：初始化为单位矩阵
    Matrix4D() { Identity(); }
    explicit Matrix4D(const std::array<std::array<double, 4>, 4>& data) : A(data) {}
    // ── 矩阵乘法 ──────────────────────────────────────────────
    Matrix4D  operator*(const Matrix4D& m) const;
    Matrix4D& operator*=(const Matrix4D& m);
    // ── 设为单位矩阵 ──────────────────────────────────────────
    void Identity();
    // ── 行列式（取左上 3×3 子矩阵）──────────────────────────
    double Determinant() const;
    // ── 工厂函数 ──────────────────────────────────────────────
    // 单位矩阵
    static Matrix4D MakeIdentity();
    // 镜像矩阵：沿过原点、法向量为 planeNormal 的平面做镜像
    static Matrix4D MakeMirror(Vector3D planeNormal);
    // 旋转矩阵：绕过原点的 axis 轴，逆时针旋转 angleRad 弧度
    static Matrix4D MakeRotation(double angleRad, Vector3D axis);
    // 均匀缩放矩阵
    static Matrix4D MakeScale(double s);
    // 平移矩阵：沿向量 v 方向平移
    static Matrix4D MakeTranslation(Vector3D v);

    void ToFloatArray(float f[16]) const;
};