#pragma once

#include <assert.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "ogldev_math_3d.h"

extern float gCurrentFOV;

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 768;

inline float ToRad(float deg) { return deg * 3.14159265358979f / 180.0f; }

inline void SetIdentity(Matrix4f& M)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M.m[i][j] = (i == j) ? 1.0f : 0.0f;
}

struct PerspectiveInfo
{
    float FOV;      // vertical field of view, in degrees
    float Width;
    float Height;
    float zNear;
    float zFar;
};

struct CameraInfo
{
    Vector3f Pos;
    Vector3f Target;   // viewing DIRECTION, not a point
    Vector3f Up;
};

class Pipeline
{
public:
    Pipeline()
    {
        m_scale      = Vector3f(1.0f, 1.0f, 1.0f);
        m_worldPos   = Vector3f(0.0f, 0.0f, 0.0f);
        m_rotateInfo = Vector3f(0.0f, 0.0f, 0.0f);

        m_persProj.FOV    = 30.0f;
        m_persProj.Width  = (float)WINDOW_WIDTH;
        m_persProj.Height = (float)WINDOW_HEIGHT;
        m_persProj.zNear  = 1.0f;
        m_persProj.zFar   = 1000.0f;

        m_camera.Pos    = Vector3f(0.0f, 0.0f, 0.0f);
        m_camera.Target = Vector3f(0.0f, 0.0f, 1.0f);
        m_camera.Up     = Vector3f(0.0f, 1.0f, 0.0f);
    }

    void SetFOV(float FOV)
    {
        m_persProj.FOV = FOV;
    }

    float GetFOV() const
    {
        return m_persProj.FOV;
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ)
    {
        m_scale = Vector3f(ScaleX, ScaleY, ScaleZ);
    }

    void WorldPos(float x, float y, float z)
    {
        m_worldPos = Vector3f(x, y, z);
    }

    void Rotate(float RotateX, float RotateY, float RotateZ)
    {
        m_rotateInfo = Vector3f(RotateX, RotateY, RotateZ);
    }

    void SetPerspectiveProj(float FOV, float Width, float Height, float zNear, float zFar)
    {
        m_persProj.FOV    = FOV;
        m_persProj.Width  = Width;
        m_persProj.Height = Height;
        m_persProj.zNear  = zNear;
        m_persProj.zFar   = zFar;
    }

    void SetCamera(const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
    {
        m_camera.Pos    = Pos;
        m_camera.Target = Target;
        m_camera.Up     = Up;
    }

    const Matrix4f* GetTrans()
    {
        Matrix4f ScaleTrans, RotateTrans, TranslationTrans;
        Matrix4f CameraTranslationTrans, CameraRotateTrans, PersProjTrans;

        ScaleTrans.InitScaleTransform(m_scale.x, m_scale.y, m_scale.z);
        RotateTrans.InitRotateTransform(m_rotateInfo.x, m_rotateInfo.y, m_rotateInfo.z);
        TranslationTrans.InitTranslationTransform(m_worldPos.x, m_worldPos.y, m_worldPos.z);
        CameraTranslationTrans.InitTranslationTransform(-m_camera.Pos.x, -m_camera.Pos.y, -m_camera.Pos.z);
        CameraRotateTrans.InitCameraTransform(m_camera.Target, m_camera.Up);
        InitPerspectiveProj(PersProjTrans);

        // right to left: scale, rotate, move into world, move world to camera, rotate to camera, project
        m_transformation = PersProjTrans * CameraRotateTrans * CameraTranslationTrans *
                           TranslationTrans * RotateTrans * ScaleTrans;
        return &m_transformation;
    }

    const Matrix4f* GetProjTrans()
    {
        InitPerspectiveProj(m_persProjTrans);
        return &m_persProjTrans;
    }

private:
    void InitPerspectiveProj(Matrix4f& M) const
    {
        const float ar         = m_persProj.Width / m_persProj.Height;
        const float zNear      = m_persProj.zNear;
        const float zFar       = m_persProj.zFar;
        const float zRange     = zNear - zFar;
        const float tanHalfFOV = tanf(ToRad(m_persProj.FOV / 2.0f));

        M.m[0][0] = 1.0f / (tanHalfFOV * ar); M.m[0][1] = 0.0f;              M.m[0][2] = 0.0f;                     M.m[0][3] = 0.0f;
        M.m[1][0] = 0.0f;                     M.m[1][1] = 1.0f / tanHalfFOV; M.m[1][2] = 0.0f;                     M.m[1][3] = 0.0f;
        M.m[2][0] = 0.0f;                     M.m[2][1] = 0.0f;              M.m[2][2] = (-zNear - zFar) / zRange; M.m[2][3] = 2.0f * zFar * zNear / zRange;
        M.m[3][0] = 0.0f;                     M.m[3][1] = 0.0f;              M.m[3][2] = 1.0f;                     M.m[3][3] = 0.0f;
    }

    Vector3f        m_scale;
    Vector3f        m_worldPos;
    Vector3f        m_rotateInfo;
    PerspectiveInfo m_persProj;
    CameraInfo      m_camera;
    Matrix4f        m_transformation;
    Matrix4f        m_persProjTrans;
};
