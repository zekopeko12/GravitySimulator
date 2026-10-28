#include "Gravity.h"
#include "ogldev_math_3d.h"
#include "Pipeline.h"
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "CBFunctions.h"
#include <cmath>

void SpecialKeyboardCB(int Key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(Key, x, y);
    if(ImGui::GetIO().WantTextInput)  return;

    gFollowing = false;
    if (pGameCamera){
        pGameCamera->StopFollowing();
        pGameCamera->SetKey(Key, true);
    }
}

void KeyboardUpCB(unsigned char Key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardUpFunc(Key, x, y);
    if(pGameCamera) pGameCamera->SetKey(Key, false);
}

void SpecialKeyboardUpCB(int Key, int x, int y)
{
    ImGui_ImplGLUT_SpecialUpFunc(Key, x, y);
    if(pGameCamera) pGameCamera->SetKey(Key, false);
}

void KeyboardCB(unsigned char Key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(Key, x, y);
    if(ImGui::GetIO().WantTextInput) return;

    if(Key == 'q' || Key == 27) { // Exit on 'q' or ESC
        exit(0);
    }
    gFollowing = false;
    if (pGameCamera){
        pGameCamera->StopFollowing();
        pGameCamera->SetKey(Key, true);
    }
}

int PickBall(int x, int y)
{
    if(!pGameCamera) return -1;

    const float w = (float)glutGet(GLUT_WINDOW_WIDTH);
    const float h = (float)glutGet(GLUT_WINDOW_HEIGHT);

    const float ndcX = 2.0f * x / w - 1.0f;
    const float ndcY = 1.0f - 2.0f * y / h;

    const float tanHalfFOV = tanf(ToRad(gCurrentFOV / 2.0f));
    const float aspect = w / h;

    Vector3f N = pGameCamera->GetTarget(); N.Normalize();
    Vector3f U = pGameCamera->GetUp().Cross(N); U.Normalize();
    Vector3f V = N.Cross(U);

    const float dx = ndcX * tanHalfFOV * aspect;
    const float dy = ndcY * tanHalfFOV;

    Vector3f D(U.x * dx + V.x * dy + N.x,
               U.y * dx + V.y * dy + N.y,
               U.z * dx + V.z * dy + N.z);
    D.Normalize();

    const Vector3f O = pGameCamera->GetPos();

    int best = -1;
    float bestT = 1e30f;

    for(int i = 0; i < (int)gBalls.size(); i++){
        const float r = ((i == gSelected) ? kSelectedBallScale : RadiusFromMass(gBalls[i].mass)) * 1.0f;

        const float ocx = O.x - gBalls[i].pos[0];
        const float ocy = O.y - gBalls[i].pos[1];
        const float ocz = O.z - gBalls[i].pos[2];

        const float b = ocx * D.x + ocy * D.y + ocz * D.z;
        const float c = ocx * ocx + ocy * ocy + ocz * ocz - r * r;
        const float disc = b * b - c;

        if(disc < 0.0f) continue;

        float t = -b - sqrtf(disc);
        if(t < 0.0f) t = -b + sqrtf(disc);
        if(t < 0.0f) continue;

        if(t < bestT){
            bestT = t;
            best = i;
        }
    }

    return best;
}

void MouseButtonCB(int button, int state, int x, int y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)x, (float)y);

    if(button >= 0 && button <= 4){
        io.MouseDown[button] = (state == GLUT_DOWN);
    }

    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !io.WantCaptureMouse){
        gSelected = PickBall(x, y);
    }

    if(button == GLUT_RIGHT_BUTTON){
        if(state == GLUT_DOWN && !io.WantCaptureMouse){
            gLookMode = true;
            if(pGameCamera) pGameCamera->ResetMouse(x, y);
            glutSetCursor(GLUT_CURSOR_NONE);
        } else if(state == GLUT_UP){
            gLookMode = false;
            if(pGameCamera) pGameCamera->ResetMouse(x, y);
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }
}

void MouseMotionCB(int x, int y)
{
    ImGui::GetIO().MousePos = ImVec2(static_cast<float>(x), static_cast<float>(y));

    if(gLookMode && pGameCamera){
        gFollowing = false;
        pGameCamera->StopFollowing();
        pGameCamera->OnMouse(x, y);
    }
}

void MouseWheelCB(int wheel, int direction, int x, int y)
{
    if(ImGui::GetIO().WantCaptureMouse) return;

    gCurrentFOV -= direction * 2.0f;

    if(gCurrentFOV < 5.0f)      gCurrentFOV = 5.0f;
    if(gCurrentFOV > 100.0f)    gCurrentFOV = 100.0f;
}

void PassiveMouseCB(int x, int y)
{
    ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);
}

void ReshapeCB(int width, int height)
{
    if(height == 0) height = 1;

    glViewport(0, 0, width, height);
    ImGui::GetIO().DisplaySize = ImVec2((float)width, (float)height);
}
