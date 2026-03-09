#pragma once
#include <cmath>
#include <stdexcept>
#include "CadBase.h"

// ============================================================
//  Vector.h
//  二维向量（Vector2D）与三维向量（Vector3D）
//
//  【坐标约定】
//    使用右手坐标系：X 向右，Y 向上，Z 向外（屏幕朝向观察者）
//
//  【行向量约定】
//    本库统一采用"行向量 × 矩阵"的乘法顺序：
//      变换后向量 = 原向量 * 矩阵
//    这与 OpenGL（列向量）相反，与 DirectX / CAD 常见习惯一致。
// ============================================================ 
 
// ============================================================
//  Vector2D —— 二维向量
//
//  表示平面内的方向与大小，不携带位置信息。
//  与 Point2D 的区别：
//    Point2D  表示"位置"，两点相减得到向量
//    Vector2D 表示"位移/方向"，可以加减、缩放、旋转
// ============================================================
class Vector2D {
public:
    double dx{};   // X 分量
    double dy{};   // Y 分量

    // ── 构造函数 ──────────────────────────────────────────────
    Vector2D() = default;
    Vector2D(double dx, double dy = 0.0) : dx(dx), dy(dy) {}

    // ── 向量加减（平行四边形法则） ────────────────────────────
    Vector2D  operator+(const Vector2D& v) const { return { dx + v.dx, dy + v.dy }; }
    Vector2D  operator-(const Vector2D& v) const { return { dx - v.dx, dy - v.dy }; }
    Vector2D& operator+=(const Vector2D& v) { dx += v.dx; dy += v.dy; return *this; }
    Vector2D& operator-=(const Vector2D& v) { dx -= v.dx; dy -= v.dy; return *this; }

    // ── 标量乘除（等比缩放向量长度） ──────────────────────────
    Vector2D  operator*(double d) const { return { dx * d, dy * d }; }
    Vector2D& operator*=(double d) { dx *= d; dy *= d; return *this; }
    Vector2D  operator/(double d) const { return { dx / d, dy / d }; }
    Vector2D& operator/=(double d) { dx /= d; dy /= d; return *this; }

    // ── 点积 (Dot Product) ────────────────────────────────────
    // 定义：v1·v2 = |v1||v2|cosθ
    // 用途：
    //   · 判断两向量夹角（> 0 同向，= 0 垂直，< 0 反向）
    //   · 计算投影长度：v1 在 v2 方向上的投影 = v1.dot(v2) / v2.length()
    double Dot(const Vector2D& v) const { return dx * v.dx + dy * v.dy; }

    // ── 叉积 Z 分量 (2D Cross Product) ───────────────────────
    // 二维叉积结果是标量（三维叉积的 Z 分量）：
    //   v1 × v2 = dx1*dy2 - dy1*dx2
    // 用途：
    //   · > 0：v2 在 v1 左侧（逆时针方向）
    //   · < 0：v2 在 v1 右侧（顺时针方向）
    //   · = 0：两向量平行
    double Cross(const Vector2D& v) const { return dx * v.dy - dy * v.dx; }

    // ── 矩阵变换 ──────────────────────────────────────────────
    // 向量变换只应用旋转/缩放，不应用平移（平移对向量无意义）
    Vector2D  operator*(const Matrix2D& m) const;
    Vector2D& operator*=(const Matrix2D& m);

    // ── 长度计算 ──────────────────────────────────────────────
    // 使用 hypot() 比手写 sqrt(dx²+dy²) 更安全（避免溢出）
    double Length() const { return std::hypot(dx, dy); }

    // ── 零向量判断 ────────────────────────────────────────────
    bool IsZero() const { return Length() <= CAD_ZERO; }

    // ── 归一化 (Normalize) ────────────────────────────────────
    // 返回与原向量同方向、长度为 1 的单位向量。
    // 零向量无法归一化，抛出异常。
    Vector2D Normalized() const {
        double len = Length();
        if (len <= CAD_ZERO)
            throw std::runtime_error("Cannot normalize zero-length vector");
        return { dx / len, dy / len };
    }
    // 原地归一化（修改自身）
    void normalize() { *this = Normalized(); }
};

// ============================================================
//  Vector3D —— 三维向量
//
//  在 Vector2D 基础上增加 Z 分量，支持空间运算。
//  叉积结果是三维向量（垂直于两输入向量的平面）。
// ============================================================
class Vector3D {
public:
    double dx{};   // X 分量
    double dy{};   // Y 分量
    double dz{};   // Z 分量 
    Vector3D() = default;
    Vector3D(double dx, double dy, double dz = 0.0) : dx(dx), dy(dy), dz(dz) {} 
    Vector3D  operator+(const Vector3D& v) const { return { dx + v.dx, dy + v.dy, dz + v.dz }; }
    Vector3D  operator-(const Vector3D& v) const { return { dx - v.dx, dy - v.dy, dz - v.dz }; }
    Vector3D& operator+=(const Vector3D& v) { dx += v.dx; dy += v.dy; dz += v.dz; return *this; }
    Vector3D& operator-=(const Vector3D& v) { dx -= v.dx; dy -= v.dy; dz -= v.dz; return *this; } 
    Vector3D  operator*(double d) const { return { dx * d, dy * d, dz * d }; }
    Vector3D& operator*=(double d) { dx *= d; dy *= d; dz *= d; return *this; }
    Vector3D  operator/(double d) const { return { dx / d, dy / d, dz / d }; }
    Vector3D& operator/=(double d) { dx /= d; dy /= d; dz /= d; return *this; } 
    double Dot(const Vector3D& v) const {
        return dx * v.dx + dy * v.dy + dz * v.dz;
    }
     
    Vector3D Cross(const Vector3D& v) const {
        return {
            dy * v.dz - dz * v.dy,
            dz * v.dx - dx * v.dz,
            dx * v.dy - dy * v.dx
        };
    }

    // ── 矩阵变换 ──────────────────────────────────────────────
    // 向量只参与旋转/缩放，忽略矩阵的平移部分
    Vector3D  operator*(const Matrix3D& m) const;
    Vector3D& operator*=(const Matrix3D& m);

    // ── 长度计算 ──────────────────────────────────────────────
    double Length()   const { return std::sqrt(dx * dx + dy * dy + dz * dz); }
    double LengthXY() const { return std::hypot(dx, dy); }   // 投影到 XY 平面的长度
    double LengthYZ() const { return std::hypot(dy, dz); }   // 投影到 YZ 平面的长度
    double LengthZX() const { return std::hypot(dz, dx); }   // 投影到 ZX 平面的长度

    // ── 零向量判断 ────────────────────────────────────────────
    bool IsZero() const { return Length() <= CAD_ZERO; }

    // ── 归一化 ────────────────────────────────────────────────
    Vector3D Normalized() const {
        double len = Length();
        if (len <= CAD_ZERO)
            throw std::runtime_error("Cannot normalize zero-length vector");
        return { dx / len, dy / len, dz / len };
    }
    void normalize() { *this = Normalized(); }
};

