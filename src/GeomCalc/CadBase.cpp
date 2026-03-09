#include "CadBase.h"
#include "Point.h"
#include "Vector.h" 

// ── 自由函数 ──────────────────────────────────────────────────

// 使用 hypot() 计算 2D 距离，比手写 sqrt 更安全（避免中间值溢出）
double DistOf(Point2D a, Point2D b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

double DistOf(Point3D a, Point3D b) {
    double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// ── angleBetween ─────────────────────────────────────────────
// 计算两向量夹角（弧度）
// 原理：cosθ = (v1·v2) / (|v1|·|v2|)
//       θ = acos(cosθ)，范围 [0, π]
// 注意：对 cosA 做 clamp 到 [-1, 1]，防止浮点误差导致 acos 返回 NaN
double AngleBetween(Vector2D v1, Vector2D v2) {
    double denom = v1.Length() * v2.Length();
    if (denom <= CAD_ZERO)
        throw std::runtime_error("Cannot compute angle with zero-length vector");
    double cosA = v1.Dot(v2) / denom;
    cosA = std::max(-1.0, std::min(1.0, cosA));   // 浮点保护
    return std::acos(cosA);
}

double AngleBetween(Vector3D v1, Vector3D v2) {
    double denom = v1.Length() * v2.Length();
    if (denom <= CAD_ZERO)
        throw std::runtime_error("Cannot compute angle with zero-length vector");
    double cosA = v1.Dot(v2) / denom;
    cosA = std::max(-1.0, std::min(1.0, cosA));
    return std::acos(cosA);
}

// ── isParallel ───────────────────────────────────────────────
// 判断两向量是否平行
// 原理：平行时叉积 = 0（|v1×v2| = |v1||v2|sinθ，θ=0 或 π 时为 0）
// 用相对阈值：|cross| <= CAD_ZERO * |v1| * |v2|，避免长向量的数值误差
bool IsParallel(Vector2D v1, Vector2D v2) {
    return std::fabs(v1.Cross(v2)) <= CAD_ZERO * v1.Length() * v2.Length();
} 

bool IsParallel(Vector3D v1, Vector3D v2) {
    // 3D：叉积结果是向量，判断其长度是否为零
    return v1.Cross(v2).IsZero();
}

// ── isOrthogonal ─────────────────────────────────────────────
// 判断两向量是否正交（垂直）
// 原理：正交时点积 = 0（v1·v2 = |v1||v2|cosθ，θ=90° 时 cosθ=0）
// 用相对阈值而非固定阈值，避免长向量的数值误差放大问题：
//   绝对阈值：|dot| <= CAD_ZERO          ← 对很长的向量会误判
//   相对阈值：|dot| <= CAD_ZERO * |v1| * |v2|  ← 与向量长度无关

bool IsOrthogonal(Vector2D v1, Vector2D v2) {
    return std::fabs(v1.Dot(v2)) <= CAD_ZERO * v1.Length() * v2.Length();
}

bool IsOrthogonal(Vector3D v1, Vector3D v2) {
    return std::fabs(v1.Dot(v2)) <= CAD_ZERO * v1.Length() * v2.Length();
}


// ── AreaOfTriangle ────────────────────────────────────────────
// 叉积法求三角形面积
//
//   v1 = vex1 - vex0
//   v2 = vex2 - vex0
//   叉积 v1 × v2 的长度 = 平行四边形面积
//   三角形面积 = |v1 × v2| / 2
double AreaOfTriangle(const Point3D& vex0,
    const Point3D& vex1,
    const Point3D& vex2) {
    Vector3D v1 = vex1 - vex0;
    Vector3D v2 = vex2 - vex0;
    return v1.Cross(v2).Length() * 0.5;
}

// ── DistOfPtLine ──────────────────────────────────────────────
// 投影法求点到直线距离
//
// 步骤：
//   1. 求 pt 到起点的向量 w = pt - sp
//   2. 将 w 投影到方向向量 vec 上，得参数 t
//      t = w·vec / |vec|²
//   3. 投影点 projPt = sp + t * vec
//   4. 距离 = |pt - projPt|
double DistOfPtLine(const Point3D& pt,
    const Point3D& sp,
    const Vector3D& vec,
    Point3D& projPt,
    double& t) {
    if (vec.IsZero())
        throw std::invalid_argument("Direction vector cannot be zero");

    Vector3D w = pt - sp;
    double lenSq = vec.Dot(vec);        // |vec|²，避免开方再平方

    t = w.Dot(vec) / lenSq;             // 投影参数
    projPt = sp + vec * t;              // 投影点
    return DistOf(pt, projPt);          // 垂直距离
}

// ── IntersectOfLines ──────────────────────────────────────────
// Cramer 法则求两直线交点
//
// 联立参数方程：
//   ptO + t0 * vec0 = ptl + t1 * vecl
//
// 整理为：
//   t0 * vec0.dx - t1 * vecl.dx = ptl.x - ptO.x
//   t0 * vec0.dy - t1 * vecl.dy = ptl.y - ptO.y
//
// 分母（行列式）：
//   D = vec0.dx * (-vecl.dy) - vec0.dy * (-vecl.dx)
//     = -(vec0 × vecl)
//
// Cramer 法则：
//   t0 = ((ptl-ptO) × vecl) / D
//   t1 = ((ptl-ptO) × vec0) / D
bool IntersectOfLines(const Point2D& ptO,
    const Vector2D& vec0,
    const Point2D& ptl,
    const Vector2D& vecl,
    Point2D& intPt,
    double& t0,
    double& t1) {
    // 分母 = vec0 × vecl（2D 叉积，即 Z 分量）
    double D = vec0.Cross(vecl);

    // D ≈ 0：两直线平行或重合，无唯一交点
    if (std::fabs(D) <= CAD_ZERO)
        return false;

    // 起点差向量
    Vector2D w = ptl - ptO;

    t0 = w.Cross(vecl) / D;
    t1 = w.Cross(vec0) / D;

    intPt = ptO + vec0 * t0;
    return true;
}

// ── CreateArcBy3P ─────────────────────────────────────────────
// 垂直平分线法求圆心
//
// 步骤：
//   1. 求 p0→pl 线段的中垂线：
//      中点 m0 = (p0 + pl) / 2
//      方向 n0 = 垂直于 (pl - p0) 的法向量
//
//   2. 求 pl→p2 线段的中垂线：
//      中点 m1 = (pl + p2) / 2
//      方向 n1 = 垂直于 (p2 - pl) 的法向量
//
//   3. 求两中垂线的交点 → 圆心 cp
//
//   4. 判断旋转方向：
//      (pl - p0) × (p2 - p0) 的 Z 分量
//        > 0：p0→pl→p2 为逆时针（CCW）
//        < 0：顺时针（CW）
bool CreateArcBy3P(const Point2D& p0,
    const Point2D& pl,
    const Point2D& p2,
    Point2D& cp,
    bool& bCCW) {
    Vector2D d0 = pl - p0;   // p0 → pl
    Vector2D d1 = p2 - pl;   // pl → p2

    // 三点共线判断：两段方向向量平行
    if (std::fabs(d0.Cross(d1)) <= CAD_ZERO)
        return false;

    // 中垂线：过线段中点，方向为线段的法向量（旋转 90°）
    // 向量 (dx, dy) 的垂直方向为 (-dy, dx)
    Point2D  m0((p0.x + pl.x) * 0.5, (p0.y + pl.y) * 0.5);
    Vector2D n0(-d0.dy, d0.dx);   // d0 的法向量

    Point2D  m1((pl.x + p2.x) * 0.5, (pl.y + p2.y) * 0.5);
    Vector2D n1(-d1.dy, d1.dx);   // d1 的法向量

    // 求两中垂线的交点
    double t0, t1;
    if (!IntersectOfLines(m0, n0, m1, n1, cp, t0, t1))
        return false;   // 理论上不会走到这里（已排除共线）

    // 判断旋转方向：(pl-p0) × (p2-p0) 的符号
    Vector2D v1 = pl - p0;
    Vector2D v2 = p2 - p0;
    bCCW = (v1.Cross(v2) > 0.0);

    return true;
}