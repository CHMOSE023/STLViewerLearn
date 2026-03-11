#include "STLWindows.h"
#include <iostream>
#include <Windows.h>
#include "Point.h"
#include "Vector.h"
#include "Matrix.h" 
#include "GeomCalc.h"
// ── 简单断言工具 ──────────────────────────────────────
static int passed = 0, failed = 0;

void check(bool cond, const char* desc) {
    if (cond) {
        std::cout << "  [PASS] " << desc << "\n";
        ++passed;
    }
    else {
        std::cout << "  [FAIL] " << desc << "\n";
        ++failed;
    }
}

bool nearEqual(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps;
}

// ── 测试函数 ──────────────────────────────────────────
void testVector2D() {
    std::cout << "\n=== Vector2D ===\n";

    Vector2D v1(3.0, 4.0);
    Vector2D v2(1.0, 2.0);

    check(nearEqual(v1.Length(), 5.0), "length (3,4) == 5");
    check(!v1.IsZero(), "非零向量 isZero() == false");
    check(Vector2D(0, 0).IsZero(), "零向量 isZero() == true");

    Vector2D sum = v1 + v2;
    check(nearEqual(sum.dx, 4.0) && nearEqual(sum.dy, 6.0), "加法 (3,4)+(1,2) == (4,6)");

    Vector2D diff = v1 - v2;
    check(nearEqual(diff.dx, 2.0) && nearEqual(diff.dy, 2.0), "减法 (3,4)-(1,2) == (2,2)");

    check(nearEqual(v1.Dot(v2), 11.0), "点积 (3,4)·(1,2) == 11");
    check(nearEqual(v1.Cross(v2), 2.0), "叉积 z分量 (3,4)×(1,2) == 2");

    Vector2D n = v1.Normalized();
    check(nearEqual(n.Length(), 1.0), "归一化后长度 == 1");
}

void testVector3D() {
    std::cout << "\n=== Vector3D ===\n";

    Vector3D v1(1.0, 0.0, 0.0);  // X轴
    Vector3D v2(0.0, 1.0, 0.0);  // Y轴

    check(nearEqual(v1.Length(), 1.0), "单位X向量 length == 1");

    Vector3D cross = v1.Cross(v2);               // X × Y = Z
    check(nearEqual(cross.dx, 0.0) &&
        nearEqual(cross.dy, 0.0) &&
        nearEqual(cross.dz, 1.0), "X × Y == Z轴");

    check(nearEqual(v1.Dot(v2), 0.0), "X · Y == 0 (正交)");
    check(IsOrthogonal(v1, v2), "isOrthogonal(X, Y) == true");
    check(IsParallel(v1, v1), "isParallel(X, X) == true");
    check(!IsParallel(v1, v2), "isParallel(X, Y) == false");

    Vector3D v3(3.0, 4.0, 0.0);
    check(nearEqual(v3.LengthXY(), 5.0), "lengthXY (3,4,0) == 5");
}

void testPoint2D() {
    std::cout << "\n=== Point2D ===\n";

    Point2D p1(1.0, 2.0);
    Point2D p2(4.0, 6.0);
    Vector2D v(3.0, 4.0);

    Point2D p3 = p1 + v;
    check(nearEqual(p3.x, 4.0) && nearEqual(p3.y, 6.0), "点 + 向量 == (4,6)");

    Vector2D diff = p2 - p1;
    check(nearEqual(diff.dx, 3.0) && nearEqual(diff.dy, 4.0), "点 - 点 == 向量 (3,4)");

    check(nearEqual(DistOf(p1, p2), 5.0), "distOf (1,2)→(4,6) == 5");
    check(p1 == Point2D(1.0, 2.0), "operator== 相等");
    check(!(p1 == p2), "operator== 不等");
}

void testPoint3D() {
    std::cout << "\n=== Point3D ===\n";

    Point3D p1(0.0, 0.0, 0.0);
    Point3D p2(1.0, 2.0, 2.0);

    check(nearEqual(DistOf(p1, p2), 3.0), "distOf (0,0,0)→(1,2,2) == 3");

    Vector3D v(1.0, 0.0, 0.0);
    Point3D p3 = p1 + v;
    check(nearEqual(p3.x, 1.0) &&
        nearEqual(p3.y, 0.0) &&
        nearEqual(p3.z, 0.0), "点 + 向量 == (1,0,0)");

    Vector3D diff = p2 - p1;
    check(nearEqual(diff.dx, 1.0) &&
        nearEqual(diff.dy, 2.0) &&
        nearEqual(diff.dz, 2.0), "点 - 点 == 向量 (1,2,2)");
}

void testMatrix2D() {
    std::cout << "\n=== Matrix2D ===\n";

    // 旋转 90°：(1,0) → (0,1)
    Matrix2D rot = Matrix2D::MakeRotation(3.14159265358979323846 / 2.0);
    Point2D p(1.0, 0.0);
    Point2D rp = p * rot;
    std::cout << "  [DEBUG] 旋转90° 实际结果: (" << rp.x << ", " << rp.y << ")\n";
    check(nearEqual(rp.x, 0.0, 1e-9) &&
        nearEqual(rp.y, 1.0, 1e-9), "旋转 90°: (1,0) → (0,1)");

    // 平移 (3,4)
    Matrix2D trans = Matrix2D::MakeTranslation(Vector2D(3.0, 4.0));
    Point2D tp = Point2D(1.0, 1.0) * trans;
    check(nearEqual(tp.x, 4.0) &&
        nearEqual(tp.y, 5.0), "平移 (3,4): (1,1) → (4,5)");

    // 缩放 x2
    Matrix2D scale = Matrix2D::MakeScale(2.0);
    Point2D sp = Point2D(3.0, 4.0) * scale;
    check(nearEqual(sp.x, 6.0) &&
        nearEqual(sp.y, 8.0), "缩放 x2: (3,4) → (6,8)");

    // 矩阵连乘：先旋转再平移
    Matrix2D combined = rot * trans;
    Point2D cp = Point2D(1.0, 0.0) * combined;
    check(nearEqual(cp.x, 3.0, 1e-9) &&
        nearEqual(cp.y, 5.0, 1e-9), "矩阵连乘：旋转90° 再平移(3,4)");
}

void testMatrix3D() {
    std::cout << "\n=== Matrix3D ===\n"; 

    // 绕 Z 轴旋转 90°：(1,0,0) → (0,1,0)
    Matrix3D rot = Matrix3D::MakeRotation(PI / 2.0, Vector3D(0, 0, 1));
    Point3D p(1.0, 0.0, 0.0);
    Point3D rp = p * rot;
    std::cout << "  [DEBUG] 3D旋转90° 实际结果: (" << rp.x << ", " << rp.y << ", " << rp.z << ")\n";
    check(nearEqual(rp.x, 0.0, 1e-9) &&
        nearEqual(rp.y, 1.0, 1e-9) &&
        nearEqual(rp.z, 0.0, 1e-9), "绕Z轴旋转90°: (1,0,0) → (0,1,0)");

    // 平移 (1,2,3)
    Matrix3D trans = Matrix3D::MakeTranslation(Vector3D(1, 2, 3));
    Point3D tp = Point3D(0, 0, 0) * trans;
    check(nearEqual(tp.x, 1.0) &&
        nearEqual(tp.y, 2.0) &&
        nearEqual(tp.z, 3.0), "平移(1,2,3): (0,0,0) → (1,2,3)");

    // 缩放 x3
    Matrix3D scale = Matrix3D::MakeScale(3.0);
    Point3D sp = Point3D(1, 2, 3) * scale;
    check(nearEqual(sp.x, 3.0) &&
        nearEqual(sp.y, 6.0) &&
        nearEqual(sp.z, 9.0), "缩放 x3: (1,2,3) → (3,6,9)");
}

void testAngles() {
    std::cout << "\n=== 角度计算 ===\n"; 

    Vector2D x2(1, 0), y2(0, 1);
    check(nearEqual(AngleBetween(x2, y2), PI / 2.0, 1e-9), "2D: X与Y夹角 == 90°");

    Vector3D x3(1, 0, 0), y3(0, 1, 0);
    check(nearEqual(AngleBetween(x3, y3), PI / 2.0, 1e-9), "3D: X与Y夹角 == 90°");

    Vector3D same(1, 1, 1);
    check(nearEqual(AngleBetween(same, same), 0.0, 1e-9), "同向向量夹角 == 0°");
}

// ─────────────────────────────────────────────────────
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    try {
        /*testVector2D();
        testVector3D();
        testPoint2D();
        testPoint3D();
        testMatrix2D();
        testMatrix3D();
        testAngles();

        std::cout << "\n─────────────────────────────────\n";
        std::cout << "结果：" << passed << " 通过，" << failed << " 失败\n";*/ 

        STLWindows* pSTLWindows = new STLWindows;
        pSTLWindows->Initialize(1200, 900, L"STLWindow");
		pSTLWindows->m_OpenGLDC->SetBackgroundColor(0.1f, 0.1f, 0.1f); 
		pSTLWindows->Run();
		delete pSTLWindows;

    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return failed == 0 ? 0 : 1;
}
