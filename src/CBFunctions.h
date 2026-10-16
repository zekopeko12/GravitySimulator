#pragma once

#include "Camera.h"
#include "Ball.h"
#include <vector>

extern Camera* pGameCamera;
extern bool gLookMode;
extern int gSelected;
extern vector<BallInstance> gBalls;
extern const float kBallScale;
extern const float kSelectedBallScale;
extern float gCurrentFOV;
extern bool gFollowing;

void SpecialKeyboardCB(int Key, int x, int y);
void KeyboardCB(unsigned char Key, int x, int y);
void SpecialKeyboardUpCB(int Key, int x, int y);
void KeyboardUpCB(unsigned char Key, int x, int y);
int PickBall(int x, int y);
void MouseButtonCB(int button, int state, int x, int y);
void MouseMotionCB(int x, int y);
void MouseWheelCB(int wheel, int direction, int x, int y);
void PassiveMouseCB(int x, int y);
void ReshapeCB(int width, int height);
