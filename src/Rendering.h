#pragma once
#include "Ball.h"
#include "Camera.h"

extern vector<BallInstance> gBalls;
extern vector<BallInstance> gAsteroids;
extern float gTimeScale;
extern const float kTrailInterval;
extern Camera* pGameCamera;
extern GLint gWVPLocation;
extern bool gFollowing;
extern float gLastFollowPos[3];
extern Ball gBall;
extern BallInstance gMoon;
extern const float outlineColor[3];
extern GLuint gShaderProgram;
extern bool gMoonInitialized;
extern float gCurrentFOV;

void SetBallTransform(int i, float scale);
void SetAsteroidTransform(int i, float scale, float width, float height);
int GetLODTier(BallInstance planet);
void RenderSceneCB();
