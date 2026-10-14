#pragma once
#include <GL/glew.h>
#include <vector>
#include "Ball.h"
#include "Camera.h"

extern GLuint gTrailVAO;
extern GLuint gTrailVBO;
extern const size_t kMaxTrailPoints;
extern const float  kTrailInterval;
extern const float kTrailFraction;
extern vector<vector<float> > gTrails;
extern vector<BallInstance> gBalls;
extern const float GravitationalConstant;
extern Camera* pGameCamera;
extern GLint gWVPLocation;
extern GLint gColorLocation;

void InitTrails();
void RecordTrails(float dt);
void DrawTrails();
