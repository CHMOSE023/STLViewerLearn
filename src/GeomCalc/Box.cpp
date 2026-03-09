#include "Box.h"
#include <algorithm>

// ============================================================
//  Box.cpp
//  包围盒实现
//
//  【normalize 的必要性】
//    构造时两个角点的大小顺序不确定，normalize() 确保
//    始终满足 x0 ≤ x1，y0 ≤ y1，z0 ≤ z1。
//    这样后续所有运算（并集、交集、包含判断）可以
//    直接用 min/max，无需再做额外判断。
// ============================================================

// ── Box2D ─────────────────────────────────────────────────────

Box2D::Box2D(double x0, double y0, double x1, double y1)
    : x0(x0), y0(y0), x1(x1), y1(y1) {
    Normalize();
}

Box2D::Box2D(Point2D p0, Point2D p1)
    : x0(p0.x), y0(p0.y), x1(p1.x), y1(p1.y) {
    Normalize();
}

// 由起点和对角线向量构造：终点 = p + v
Box2D::Box2D(Point2D p, Vector2D v)
    : x0(p.x), y0(p.y), x1(p.x + v.dx), y1(p.y + v.dy) {
    Normalize();
}

void Box2D::Normalize() {
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);
}

// ── 并集 ──────────────────────────────────────────────────────
// 原理：取各轴的最小下界和最大上界
//   x0_new = min(x0_a, x0_b)
//   x1_new = max(x1_a, x1_b)
Box2D Box2D::operator+(const Box2D& b) const {
    return {
        std::min(x0, b.x0), std::min(y0, b.y0),
        std::max(x1, b.x1), std::max(y1, b.y1)
    };
}
Box2D& Box2D::operator+=(const Box2D& b) { return *this = *this + b; }

// ── 交集 ──────────────────────────────────────────────────────
// 原理：取各轴的最大下界和最小上界
//   x0_new = max(x0_a, x0_b)
//   x1_new = min(x1_a, x1_b)
// 若 x0_new > x1_new，则该轴无重叠，结果退化（isEmpty() == true）
Box2D Box2D::operator&(const Box2D& b) const {
    return {
        std::max(x0, b.x0), std::max(y0, b.y0),
        std::min(x1, b.x1), std::min(y1, b.y1)
    };
}
Box2D& Box2D::operator&=(const Box2D& b) { return *this = *this & b; }

// ── 平移 ──────────────────────────────────────────────────────
Box2D Box2D::operator+(const Vector2D& v) const {
    return { x0 + v.dx, y0 + v.dy, x1 + v.dx, y1 + v.dy };
}
Box2D& Box2D::operator+=(const Vector2D& v) { return *this = *this + v; }

Box2D Box2D::operator-(const Vector2D& v) const {
    return { x0 - v.dx, y0 - v.dy, x1 - v.dx, y1 - v.dy };
}
Box2D& Box2D::operator-=(const Vector2D& v) { return *this = *this - v; }

// ── 几何属性 ──────────────────────────────────────────────────
double Box2D::Width()  const { return x1 - x0; }
double Box2D::Height() const { return y1 - y0; }
double Box2D::Area()   const { return Width() * Height(); }

Point2D Box2D::Center() const {
    return { (x0 + x1) * 0.5, (y0 + y1) * 0.5 };
}

// 退化条件：宽或高小于精度阈值
bool Box2D::IsEmpty() const {
    return Width() < CAD_ZERO || Height() < CAD_ZERO;
}

// ── 空间关系 ──────────────────────────────────────────────────

// 点在包围盒内：各轴均在 [min, max] 区间内
bool Box2D::Contains(Point2D p) const {
    return p.x >= x0 && p.x <= x1
        && p.y >= y0 && p.y <= y1;
}

// 包围盒包含另一个包围盒：各轴下界 ≥ 自身下界，上界 ≤ 自身上界
bool Box2D::Contains(const Box2D& b) const {
    return b.x0 >= x0 && b.x1 <= x1
        && b.y0 >= y0 && b.y1 <= y1;
}

// 与另一包围盒的关系
// Separated：任意轴上完全不重叠
// Contained：完全包含
// Intersected：部分重叠
Box2D::Relation Box2D::RelationWith(const Box2D& b) const {
    // 分离判断：任一轴上一方的最大值 < 另一方的最小值
    if (x1 < b.x0 || b.x1 < x0 ||
        y1 < b.y0 || b.y1 < y0)
        return Separated;

    if (Contains(b) || b.Contains(*this))
        return Contained;

    return Intersected;
}

// ══════════════════════════════════════════════════════════════
//  Box3D
// ══════════════════════════════════════════════════════════════

Box3D::Box3D(double x0, double y0, double z0,
    double x1, double y1, double z1)
    : x0(x0), y0(y0), z0(z0), x1(x1), y1(y1), z1(z1) {
    Normalize();
}

Box3D::Box3D(Point3D p0, Point3D p1)
    : x0(p0.x), y0(p0.y), z0(p0.z),
    x1(p1.x), y1(p1.y), z1(p1.z) {
    Normalize();
}

Box3D::Box3D(Point3D p, Vector3D v)
    : x0(p.x), y0(p.y), z0(p.z),
    x1(p.x + v.dx), y1(p.y + v.dy), z1(p.z + v.dz) {
    Normalize();
}

void Box3D::Normalize() {
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);
    if (z0 > z1) std::swap(z0, z1);
}

// ── 并集 ──────────────────────────────────────────────────────
Box3D Box3D::operator+(const Box3D& b) const {
    return {
        std::min(x0, b.x0), std::min(y0, b.y0), std::min(z0, b.z0),
        std::max(x1, b.x1), std::max(y1, b.y1), std::max(z1, b.z1)
    };
}
Box3D& Box3D::operator+=(const Box3D& b) { return *this = *this + b; }

// ── 交集 ──────────────────────────────────────────────────────
Box3D Box3D::operator&(const Box3D& b) const {
    return {
        std::max(x0, b.x0), std::max(y0, b.y0), std::max(z0, b.z0),
        std::min(x1, b.x1), std::min(y1, b.y1), std::min(z1, b.z1)
    };
}
Box3D& Box3D::operator&=(const Box3D& b) { return *this = *this & b; }

// ── 平移 ──────────────────────────────────────────────────────
Box3D Box3D::operator+(const Vector3D& v) const {
    return { x0 + v.dx, y0 + v.dy, z0 + v.dz, x1 + v.dx, y1 + v.dy, z1 + v.dz };
}
Box3D& Box3D::operator+=(const Vector3D& v) { return *this = *this + v; }

Box3D Box3D::operator-(const Vector3D& v) const {
    return { x0 - v.dx, y0 - v.dy, z0 - v.dz, x1 - v.dx, y1 - v.dy, z1 - v.dz };
}
Box3D& Box3D::operator-=(const Vector3D& v) { return *this = *this - v; }

// ── 均匀缩放（以原点为中心）──────────────────────────────────
Box3D Box3D::operator*(double s) const {
    return { x0 * s, y0 * s, z0 * s, x1 * s, y1 * s, z1 * s };
}
Box3D& Box3D::operator*=(double s) { return *this = *this * s; }

// ── 几何属性 ──────────────────────────────────────────────────
double Box3D::Width()  const { return x1 - x0; }   // X 方向
double Box3D::Length() const { return y1 - y0; }   // Y 方向
double Box3D::Height() const { return z1 - z0; }   // Z 方向
double Box3D::Volume() const { return Width() * Length() * Height(); }

Point3D Box3D::Center() const {
    return { (x0 + x1) * 0.5, (y0 + y1) * 0.5, (z0 + z1) * 0.5 };
}

bool Box3D::IsEmpty() const {
    return Width() < CAD_ZERO
        || Length() < CAD_ZERO
        || Height() < CAD_ZERO;
}

// ── 空间关系 ──────────────────────────────────────────────────
bool Box3D::Contains(Point3D p) const {
    return p.x >= x0 && p.x <= x1
        && p.y >= y0 && p.y <= y1
        && p.z >= z0 && p.z <= z1;
}

bool Box3D::Contains(const Box3D& b) const {
    return b.x0 >= x0 && b.x1 <= x1
        && b.y0 >= y0 && b.y1 <= y1
        && b.z0 >= z0 && b.z1 <= z1;
}

Box3D::Relation Box3D::RelationWith(const Box3D& b) const {
    // 任意轴分离 → 完全不相交
    if (x1 < b.x0 || b.x1 < x0 ||
        y1 < b.y0 || b.y1 < y0 ||
        z1 < b.z0 || b.z1 < z0)
        return Separated;

    if (Contains(b) || b.Contains(*this))
        return Contained;

    return Intersected;
}