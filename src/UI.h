#pragma once

#include "Ball.h"

extern int gSelected;
extern float gTimeScale;
extern bool gFollowing;

void RenderMenus(int height, int width, std::vector<BallInstance>& gBalls, int& gSelected, vector<BallInstance>& gAsteroids);
void RenderPlanetCreatorMenu(vector<BallInstance>& gBalls, int& gSelected, bool& showPlanetCreator);
