#include "ogldev_math_3d.h"
#include "Pipeline.h"

Vector3f Vector3f::Cross(const Vector3f& v) const
{
    return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vector3f& Vector3f::Normalize()
{
    const float Length = sqrtf(x * x + y * y + z * z);
    x /= Length; y /= Length; z /= Length;
    return *this;
}

void Matrix4f::InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ)
{
    SetIdentity(*this);
    m[0][0] = ScaleX; m[1][1] = ScaleY; m[2][2] = ScaleZ;
}

void Matrix4f::InitTranslationTransform(float x, float y, float z)
{
    SetIdentity(*this);
    m[0][3] = x; m[1][3] = y; m[2][3] = z;
}

void Matrix4f::InitRotateTransform(float RotateX, float RotateY, float RotateZ)
{
    const float x = ToRad(RotateX);
    const float y = ToRad(RotateY);
    const float z = ToRad(RotateZ);

    Matrix4f rx, ry, rz;
    SetIdentity(rx); SetIdentity(ry); SetIdentity(rz);

    rx.m[1][1] =  cosf(x); rx.m[1][2] = -sinf(x);
    rx.m[2][1] =  sinf(x); rx.m[2][2] =  cosf(x);

    ry.m[0][0] =  cosf(y); ry.m[0][2] = -sinf(y);
    ry.m[2][0] =  sinf(y); ry.m[2][2] =  cosf(y);

    rz.m[0][0] =  cosf(z); rz.m[0][1] = -sinf(z);
    rz.m[1][0] =  sinf(z); rz.m[1][1] =  cosf(z);

    *this = rz * ry * rx;
}

void Matrix4f::InitCameraTransform(const Vector3f& Target, const Vector3f& Up)
{
    Vector3f N = Target; N.Normalize();
    Vector3f U = Up; U = U.Cross(N); U.Normalize();
    Vector3f V = N.Cross(U);

    m[0][0] = U.x;  m[0][1] = U.y;  m[0][2] = U.z;  m[0][3] = 0.0f;
    m[1][0] = V.x;  m[1][1] = V.y;  m[1][2] = V.z;  m[1][3] = 0.0f;
    m[2][0] = N.x;  m[2][1] = N.y;  m[2][2] = N.z;  m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}
