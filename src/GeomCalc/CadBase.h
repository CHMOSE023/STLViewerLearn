#pragma once

// ============================================================
//  CadBase.h
//  全局常量与宏定义，被所有几何模块共用
// ============================================================

// ── 精度阈值 ─────────────────────────────────────────────────
// CAD_ZERO：几何计算的零判断阈值（1e-6）
//   用于浮点数比较，避免直接写 == 0。
//   例如：两点距离 < CAD_ZERO 时认为重合。
#define CAD_ZERO          1.0E-6

// NC_ZERO：数控加工场景下的零判断阈值（1e-3）
//   比 CAD_ZERO 宽松，适用于刀路、公差等允许更大误差的场合。
#define NC_ZERO           1.0E-3

// ── 常用判断宏 ────────────────────────────────────────────────
// IS_ZERO(x)：判断 x 是否在 CAD 精度内为零
#define IS_ZERO(x)        (fabs(x) <= CAD_ZERO)

// IS_NCZERO(x)：判断 x 是否在数控精度内为零
#define IS_NCZERO(x)      (fabs(x) <= NC_ZERO)

// IS_BETWEEN(x, min, max)：判断 x 是否在 [min, max] 区间内（含端点）
#define IS_BETWEEN(x,min,max) (x <= max && x >= min)
#define PI                3.14159265358979323846

// ── 前向声明 ──────────────────────────────────────────────────
// 各几何类在此统一声明，供各头文件互相引用时使用，
// 避免循环 include。
class Point2D;
class Point3D;
class Vector2D;
class Vector3D;
class Matrix2D;
class Matrix3D;
class Box2D;
class Box3D;

// 计算两点之间的欧氏距离
// 2D: sqrt((x1-x0)² + (y1-y0)²)
// 3D: sqrt((x1-x0)² + (y1-y0)² + (z1-z0)²)
double DistOf(Point2D a, Point2D b);
double DistOf(Point3D a, Point3D b);

// 判断两向量是否平行（同向或反向） 
bool IsParallel(Vector2D v1, Vector2D v2);
bool IsParallel(Vector3D v1, Vector3D v2);

// 判断两向量是否正交（垂直） 
bool IsOrthogonal(Vector2D v1, Vector2D v2);
bool IsOrthogonal(Vector3D v1, Vector3D v2);

// 计算两向量夹角（弧度，范围 [0, π]）
// 原理：cosθ = (v1·v2) / (|v1|·|v2|)
double AngleBetween(Vector2D v1, Vector2D v2);
double AngleBetween(Vector3D v1, Vector3D v2);

// 计算三角形面积 
double AreaOfTriangle(const Point3D & vex0, const Point3D& vex1, const Point3D& vex2);

// 点到直线的距离
double DistOfPtLine(const Point3D& pt, const  Point3D& sp, const Vector3D& vec, Point3D& projPt, double& t);

// 两直线交点 
bool  IntersectOfLines(const Point2D& ptO, const Vector2D& vec0, const Point2D& ptl, const Vector2D& vecl, Point2D& intPt, double& t0, double& t1);

// 三点确定圆弧
bool  CreateArcBy3P(const Point2D& p0, const Point2D& pl, const Point2D& p2, Point2D& cp, bool& bCCW);
