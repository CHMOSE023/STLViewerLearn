#pragma once
#include "CadBase.h"
#include "Point.h"
#include "Vector.h"

// ============================================================
//  Box.h
//  二维包围盒（Box2D）与三维包围盒（Box3D）
//
//  【包围盒的作用】
//    包围盒是包裹几何对象的最小轴对齐矩形/长方体。
//    在 CAD / 图形系统中大量用于：
//      · 快速碰撞检测：先判断包围盒是否相交，再做精确计算
//      · 视口裁剪：判断对象是否在视野范围内
//      · 空间索引：加速几何查询（如 BVH、八叉树）
//
//  【轴对齐（AABB）】
//    Axis-Aligned Bounding Box，各边平行于坐标轴。
//    优点：计算简单、快速。
//    缺点：旋转后包围盒会变大（不是最优包围）。
//
//  【坐标约定】
//    (x0, y0, z0)：最小角点（左下角/前下角）
//    (x1, y1, z1)：最大角点（右上角/后上角）
//    始终保持 x0 ≤ x1，y0 ≤ y1，z0 ≤ z1
// ============================================================

// ============================================================
//  Box2D —— 二维轴对齐包围盒
// ============================================================
class Box2D {
public:
    double x0{};   // 左边界（X 最小值）
    double y0{};   // 下边界（Y 最小值）
    double x1{};   // 右边界（X 最大值）
    double y1{};   // 上边界（Y 最大值）

    // ── 构造函数 ──────────────────────────────────────────────
    Box2D() = default;
    Box2D(double x0, double y0, double x1, double y1);

    // 由两个角点构造（自动处理大小顺序）
    Box2D(Point2D p0, Point2D p1);

    // 由起点和尺寸向量构造：p 为原点，v 为对角线向量
    Box2D(Point2D p, Vector2D v);

    // ── 并集：包含两个包围盒的最小包围盒 ────────────────────
    // 用途：逐步扩展包围盒以包含更多对象
    Box2D  operator+(const Box2D& b) const;
    Box2D& operator+=(const Box2D& b);

    // ── 交集：两个包围盒的重叠区域 ───────────────────────────
    // 若不相交，结果为退化盒（IsEmpty() == true）
    Box2D  operator&(const Box2D& b) const;
    Box2D& operator&=(const Box2D& b);

    // ── 平移 ──────────────────────────────────────────────────
    Box2D  operator+(const Vector2D& v) const;
    Box2D& operator+=(const Vector2D& v);
    Box2D  operator-(const Vector2D& v) const;
    Box2D& operator-=(const Vector2D& v);

    // ── 几何属性 ──────────────────────────────────────────────
    double Width()  const;   // X 方向长度
    double Height() const;   // Y 方向长度
    double Area()   const;   // 面积

    // 中心点
    Point2D Center() const;

    // 是否退化（宽或高为零）
    bool IsEmpty() const;

    // ── 空间关系 ──────────────────────────────────────────────
    // 判断点是否在包围盒内（含边界）
    bool Contains(Point2D p) const;

    // 判断另一个包围盒是否完全在此包围盒内
    bool Contains(const Box2D& b) const;

    // 判断与另一个包围盒的关系
    enum Relation { Separated, Intersected, Contained };
    Relation RelationWith(const Box2D& b) const;

private:
    // 确保 x0 ≤ x1，y0 ≤ y1
    void Normalize();
};

// ============================================================
//  Box3D —— 三维轴对齐包围盒
// ============================================================
class Box3D {
public:
    double x0{};   // X 最小值
    double y0{};   // Y 最小值
    double z0{};   // Z 最小值
    double x1{};   // X 最大值
    double y1{};   // Y 最大值
    double z1{};   // Z 最大值

    // ── 构造函数 ──────────────────────────────────────────────
    Box3D() = default;
    Box3D(double x0, double y0, double z0, double x1, double y1, double z1);

    // 由两个角点构造
    Box3D(Point3D p0, Point3D p1);

    // 由起点和尺寸向量构造
    Box3D(Point3D p, Vector3D v);

    // ── 并集 ──────────────────────────────────────────────────
    Box3D  operator+(const Box3D& b) const;
    Box3D& operator+=(const Box3D& b);

    // ── 交集 ──────────────────────────────────────────────────
    Box3D  operator&(const Box3D& b) const;
    Box3D& operator&=(const Box3D& b);

    // ── 平移 ──────────────────────────────────────────────────
    Box3D  operator+(const Vector3D& v) const;
    Box3D& operator+=(const Vector3D& v);
    Box3D  operator-(const Vector3D& v) const;
    Box3D& operator-=(const Vector3D& v);

    // ── 均匀缩放（以原点为中心）──────────────────────────────
    Box3D  operator*(double s) const;
    Box3D& operator*=(double s);

    // ── 几何属性 ──────────────────────────────────────────────
    double Width()  const;    // X 方向长度
    double Length() const;    // Y 方向长度
    double Height() const;    // Z 方向长度
    double Volume() const;    // 体积

    // 中心点
    Point3D Center() const;

    // 是否退化（任意方向长度为零）
    bool IsEmpty() const;

    // ── 空间关系 ──────────────────────────────────────────────
    // 判断点是否在包围盒内（含边界）
    bool Contains(Point3D p) const;

    // 判断另一个包围盒是否完全在此包围盒内
    bool Contains(const Box3D& b) const;

    // 判断与另一个包围盒的关系
    enum Relation { Separated, Intersected, Contained };
    Relation RelationWith(const Box3D& b) const;

private:
    void Normalize();
};