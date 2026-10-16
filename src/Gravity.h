#pragma once
#include "Ball.h"
#include <vector>

extern vector<BallInstance> gBalls;
extern vector<BallInstance> gAsteroids;
extern vector<vector<float> > gTrails;
extern const float GravitationalConstant;
extern const float kPlanetMassScale;
extern BallInstance gMoon;
extern bool gMoonInitialized;

void StepGravity(float dt, float maxStep = 0.02f);
void SolarSystemDemo();
int FindHeaviestBall();
float RadiusFromMass(float mass);
void ResolveCollision(int ball1, int ball2);
void AsteroidPlaneSetup(BallInstance sun, float& px, float& pz);
