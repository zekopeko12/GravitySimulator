#pragma once

#include <math.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "ogldev_math_3d.h"

inline float RadToDeg(float rad) { return rad * 180.0f / 3.14159265358979f; }

inline Vector3f RotateAroundAxis(const Vector3f& v, float AngleDeg, const Vector3f& Axis)
{
    const float a = AngleDeg * 3.14159265358979f / 180.0f;
    const float c = cosf(a);
    const float s = sinf(a);

    const float dot = Axis.x * v.x + Axis.y * v.y + Axis.z * v.z;
    const float cx  = Axis.y * v.z - Axis.z * v.y;   // Axis x v
    const float cy  = Axis.z * v.x - Axis.x * v.z;
    const float cz  = Axis.x * v.y - Axis.y * v.x;

    return Vector3f(v.x * c + cx * s + Axis.x * dot * (1.0f - c),
                    v.y * c + cy * s + Axis.y * dot * (1.0f - c),
                    v.z * c + cz * s + Axis.z * dot * (1.0f - c));
}

class Camera
{
public:
    Camera(int WindowWidth, int WindowHeight,
           const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up)
    {
        m_windowWidth  = WindowWidth;
        m_windowHeight = WindowHeight;
        m_stepSize     = 0.5f;

        m_pos = Pos;
        m_targetPos = m_pos;

        m_target = Target;
        m_target.Normalize();

        m_up = Up;
        m_up.Normalize();

        Init();
    }

    const Vector3f& GetPos() const    { return m_pos; }
    const Vector3f& GetTarget() const { return m_target; }
    const Vector3f& GetUp() const     { return m_up; }

    void ResetMouse(int x, int y)
    {
        m_mouseX = x;
        m_mouseY = y;
        m_OnLeftEdge = m_OnRightEdge = m_OnUpperEdge = m_OnLowerEdge = false;
    }

    void StopFollowing()
    {
        m_targetPos = m_pos;
    }

    bool OnKeyboard(int Key)
    {
        bool Ret = false;
        Vector3f Left = m_target.Cross(m_up);
        Left.Normalize();
        Vector3f Right = m_up.Cross(m_target);
        Right.Normalize();

        switch (Key) {

        case GLUT_KEY_UP:
            m_pos.x += m_target.x * m_stepSize;
            m_pos.y += m_target.y * m_stepSize;
            m_pos.z += m_target.z * m_stepSize;
            Ret = true;
            break;

        case GLUT_KEY_DOWN:
            m_pos.x -= m_target.x * m_stepSize;
            m_pos.y -= m_target.y * m_stepSize;
            m_pos.z -= m_target.z * m_stepSize;
            Ret = true;
            break;

        case GLUT_KEY_LEFT:
        {
            m_pos.x += Left.x * m_stepSize;
            m_pos.y += Left.y * m_stepSize;
            m_pos.z += Left.z * m_stepSize;
            Ret = true;
        }
        break;

        case GLUT_KEY_RIGHT:
        {
            m_pos.x += Right.x * m_stepSize;
            m_pos.y += Right.y * m_stepSize;
            m_pos.z += Right.z * m_stepSize;
            Ret = true;
        }
        break;
        }

        return Ret;
    }

    void OnMouse(int x, int y)
    {
        const int DeltaX = x - m_mouseX;
        const int DeltaY = y - m_mouseY;

        m_mouseX = x;
        m_mouseY = y;

        m_AngleH += (float)DeltaX / 20.0f;
        m_AngleV += (float)DeltaY / 20.0f;

        // keep looking up/down within +-90 degrees
        if (m_AngleV >  90.0f) m_AngleV =  90.0f;
        if (m_AngleV < -90.0f) m_AngleV = -90.0f;

        // at the window border the mouse can't move further, so keep turning
        if (DeltaX == 0) {
            if (x <= MARGIN) {
                m_OnLeftEdge = true;
            }
            else if (x >= (m_windowWidth - MARGIN)) {
                m_OnRightEdge = true;
            }
        }
        else {
            m_OnLeftEdge  = false;
            m_OnRightEdge = false;
        }

        if (DeltaY == 0) {
            if (y <= MARGIN) {
                m_OnUpperEdge = true;
            }
            else if (y >= (m_windowHeight - MARGIN)) {
                m_OnLowerEdge = true;
            }
        }
        else {
            m_OnUpperEdge = false;
            m_OnLowerEdge = false;
        }

        Update();
    }

    void OnRender()
    {
        bool ShouldUpdate = false;

        if (m_OnLeftEdge) {
            m_AngleH -= 0.1f;
            ShouldUpdate = true;
        }
        else if (m_OnRightEdge) {
            m_AngleH += 0.1f;
            ShouldUpdate = true;
        }

        if (m_OnUpperEdge) {
            if (m_AngleV > -90.0f) {
                m_AngleV -= 0.1f;
                ShouldUpdate = true;
            }
        }
        else if (m_OnLowerEdge) {
            if (m_AngleV < 90.0f) {
                m_AngleV += 0.1f;
                ShouldUpdate = true;
            }
        }

        if (ShouldUpdate) {
            Update();
        }
    }

    void SetTargetPosition(const Vector3f& pos)
    {
        m_targetPos = pos;
    }

    void UpdateSmoothMovement(float dt)
    {
        float smoothFactor = 8.0f * dt;

        m_pos.x += (m_targetPos.x - m_pos.x) * smoothFactor;
        m_pos.y += (m_targetPos.y - m_pos.y) * smoothFactor;
        m_pos.z += (m_targetPos.z - m_pos.z) * smoothFactor;
    }

    void LookAt(const Vector3f& Point)
    {
        Vector3f d(Point.x - m_pos.x, Point.y - m_pos.y, Point.z - m_pos.z);
        const float len = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
        if(len < 0.001f) return;

        m_target = d;
        m_target.Normalize();
        m_up = Vector3f(0.0f, 1.0f, 0.0f);

        SyncAngles();
    }

private:
    void Init()
    {
        SyncAngles();

        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
        m_OnLeftEdge  = false;
        m_OnRightEdge = false;

        m_mouseX = m_windowWidth / 2;
        m_mouseY = m_windowHeight / 2;

        glutWarpPointer(m_mouseX, m_mouseY);
    }

    void SyncAngles()
    {
        Vector3f HTarget(m_target.x, 0.0f, m_target.z);
        HTarget.Normalize();

        if (HTarget.z >= 0.0f) {
            if (HTarget.x >= 0.0f) {
                m_AngleH = 360.0f - RadToDeg(asinf(HTarget.z));
            }
            else {
                m_AngleH = 180.0f + RadToDeg(asinf(HTarget.z));
            }
        }
        else {
            if (HTarget.x >= 0.0f) {
                m_AngleH = RadToDeg(asinf(-HTarget.z));
            }
            else {
                m_AngleH = 180.0f - RadToDeg(asinf(-HTarget.z));
            }
        }

        // vertical angle
        m_AngleV = -RadToDeg(asinf(m_target.y));
    }

    void Update()
    {
        const Vector3f Vaxis(0.0f, 1.0f, 0.0f);

        // turn left/right around the vertical axis
        Vector3f View = RotateAroundAxis(Vector3f(1.0f, 0.0f, 0.0f), m_AngleH, Vaxis);
        View.Normalize();

        // tilt up/down around the camera's horizontal axis
        Vector3f Haxis = Vaxis.Cross(View);
        Haxis.Normalize();
        View = RotateAroundAxis(View, m_AngleV, Haxis);
        View.Normalize();

        m_target = View;
        m_target.Normalize();

        m_up = m_target.Cross(Haxis);
        m_up.Normalize();
    }

    static const int MARGIN = 40;   // pixels from the window edge that count as "the edge"

    Vector3f m_pos;
    Vector3f m_target;   // normalized viewing DIRECTION
    Vector3f m_targetPos;
    Vector3f m_up;
    float    m_stepSize;

    int   m_windowWidth;
    int   m_windowHeight;
    int   m_mouseX;
    int   m_mouseY;
    float m_AngleH;
    float m_AngleV;
    bool  m_OnUpperEdge;
    bool  m_OnLowerEdge;
    bool  m_OnLeftEdge;
    bool  m_OnRightEdge;
};
