#pragma once
#include "CadBase.h"
#include <cmath>

// ============================================================
//  Point.h
//  二维点（Point2D）与三维点（Point3D）
//
//  【点与向量的区别】
//    Point  表示空间中的"位置"，有坐标原点的概念
//    Vector 表示"位移/方向"，与位置无关
//
//  【合法运算】
//    点 + 向量 = 点    （将点沿向量方向移动）
//    点 - 向量 = 点    （反方向移动）
//    点 - 点   = 向量  （两点之差得到位移向量）
//    点 * 矩阵 = 点    （含平移的完整变换）
//
//  【非法运算（未提供）】
//    点 + 点           （几何上无意义）
//    向量 * 矩阵平移    （向量不受平移影响，见 Vector.h）
// ============================================================

// ============================================================
//  Point2D —— 二维点
// ============================================================
class Point2D {
public:
    double x{};   // X 坐标
    double y{};   // Y 坐标

    // ── 构造函数 ──────────────────────────────────────────────
    Point2D() = default;
    Point2D(double x, double y) : x(x), y(y) {}

    // ── 点 ± 向量 → 点 ───────────────────────────────────────
    // 将点沿向量方向平移，结果仍是点
    Point2D  operator+(const Vector2D& v) const;
    Point2D  operator-(const Vector2D& v) const;
    Point2D& operator+=(const Vector2D& v);
    Point2D& operator-=(const Vector2D& v);

    // ── 点 - 点 → 向量 ───────────────────────────────────────
    // 返回从 p 指向 *this 的位移向量
    // 含义：要从 p 走到 *this，需要走多少（方向 + 距离）
    Vector2D operator-(const Point2D& p) const;

    // ── 点 × 矩阵 → 点 ───────────────────────────────────────
    // 应用齐次变换（旋转 + 缩放 + 平移）
    // 行向量形式：[x, y, 1] × M，取前两个分量作为新坐标
    Point2D  operator*(const Matrix2D& m) const;
    Point2D& operator*=(const Matrix2D& m);

    // ── 相等判断 ──────────────────────────────────────────────
    // C++20 默认生成逐成员比较（精确比较，非近似）
    // 注意：浮点精确相等较严格，需精确相等时使用；
    //       近似相等请用 distOf(a, b) < CAD_ZERO
    bool operator==(const Point2D&) const = default;
};

// ============================================================
//  Point3D —— 三维点
// ============================================================
class Point3D {
public:
    double x{};   // X 坐标
    double y{};   // Y 坐标
    double z{};   // Z 坐标

    // ── 构造函数 ──────────────────────────────────────────────
    Point3D() = default;
    Point3D(double x, double y, double z = 0.0) : x(x), y(y), z(z) {}

    // ── 点 ± 向量 → 点 ───────────────────────────────────────
    Point3D  operator+(const Vector3D& v) const;
    Point3D  operator-(const Vector3D& v) const;
    Point3D& operator+=(const Vector3D& v);
    Point3D& operator-=(const Vector3D& v);

    // ── 点 - 点 → 向量 ───────────────────────────────────────
    // 返回从 p 指向 *this 的三维位移向量
    Vector3D operator-(const Point3D& p) const;

    // ── 点 × 矩阵 → 点 ───────────────────────────────────────
    // 行向量形式：[x, y, z, 1] × M，取前三个分量作为新坐标
    Point3D  operator*(const Matrix3D& m) const;
    Point3D& operator*=(const Matrix3D& m);

    // ── 相等 / 不等判断 ───────────────────────────────────────
    bool operator==(const Point3D&) const = default;
    bool operator!=(const Point3D&) const = default;
};
 