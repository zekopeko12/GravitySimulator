#pragma once
#include "Ball.h"
#include <vector>

extern vector<BallInstance> gBalls;
extern vector<vector<float> > gTrails;
extern const float GravitationalConstant;
extern const float kPlanetMassScale;

void StepGravity(float dt);
void SolarSystemDemo();
int FindHeaviestBall();
float RadiusFromMass(float mass);
void ResolveCollision(int ball1, int ball2);
