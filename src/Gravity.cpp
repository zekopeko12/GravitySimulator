#include "Gravity.h"
#include "Ball.h"
#include "CBFunctions.h"

void StepGravity(float dt)
{
    const int n = (int)gBalls.size();
    const float maxStep = 0.002f;
    int substeps = (int)ceilf(dt / maxStep);
    if(substeps < 1) substeps = 1;

    const float h = dt / substeps;
    const float softening = 0.01f;

    vector<float> acc(n * 3);

    for(int s = 0; s < substeps; s++){
        fill(acc.begin(), acc.end(), 0.0f);

        for(int i = 0; i < n; i++){
            for(int j = 0; j < n; j++){
                if(i == j) continue;

                const float dx = gBalls[j].pos[0] - gBalls[i].pos[0];
                const float dy = gBalls[j].pos[1] - gBalls[i].pos[1];
                const float dz = gBalls[j].pos[2] - gBalls[i].pos[2];

                const float d2   = dx * dx + dy * dy + dz * dz + softening;
                const float inv3 = 1.0f / (d2 * sqrtf(d2));
                const float k    = GravitationalConstant * gBalls[j].mass * inv3;

                acc[i * 3 + 0] += k * dx;
                acc[i * 3 + 1] += k * dy;
                acc[i * 3 + 2] += k * dz;
            }
        }

        for(int i = 0; i < n; i++){
            for(int c = 0; c < 3; c++){
                gBalls[i].vel[c] += acc[i * 3 + c] * h;
                gBalls[i].pos[c] += gBalls[i].vel[c] * h;
            }
        }

        for(int i = 0; i < n; i++){
            if(gBalls[i].dead)  continue;

            for(int j = i + 1; j < n; j++){
                if(gBalls[j].dead)  continue;

                ResolveCollision(i, j);
            }
        }
    }
}

void SolarSystemDemo()
{
    gBalls.clear();
    gTrails.clear();

    const float center[3] = { 0.0f, 0.0f, 0.8f };
    const float kAUScale = 8.0f;

    BallInstance sun;
    sun.pos[0] = center[0]; sun.pos[1] = center[1]; sun.pos[2] = center[2];
    sun.color[0] = sun.color[1] = sun.color[2] = 0.0f;
    sun.mass = 333000.0f;
    gBalls.push_back(sun);

    BallInstance planets[8];
    const float masses[8] = { 0.055f, 0.815f, 1.0f, 0.107f, 317.8f, 95.2f, 14.5f, 17.1f };
    const float au[8]     = { 0.39f, 0.72f, 1.0f, 1.52f, 5.20f, 9.58f, 19.20f, 30.05f };
    const float angles[8] = { 0.0f, 2.1f, 4.2f, 0.0f, 2.1f, 4.2f, 0.0f, 2.1f };
    const float colors[8][3] = {
            {0.7f, 0.7f, 0.7f},   // Mercury
            {0.9f, 0.7f, 0.3f},   // Venus
            {0.2f, 0.5f, 1.0f},   // Earth
            {0.9f, 0.3f, 0.2f},   // Mars
            {0.9f, 0.6f, 0.4f},   // Jupiter
            {0.9f, 0.8f, 0.5f},   // Saturn
            {0.5f, 0.9f, 0.9f},   // Uranus
            {0.3f, 0.3f, 0.9f}    // Neptune
        };

    float px = 0.0f, pz = 0.0f;

    for(int i = 0; i < 8; i++){
        BallInstance b;
        const float r = au[i] * kAUScale;
        const float a = angles[i];
        const float v = sqrtf(GravitationalConstant * sun.mass / r);

        b.pos[0] = center[0] + r * cosf(a);
        b.pos[1] = center[1];
        b.pos[2] = center[2] + r * sinf(a);

        b.vel[0] = -v * sinf(a);
        b.vel[1] = 0.0f;
        b.vel[2] = v * cosf(a);

        b.mass = masses[i] * kPlanetMassScale;
        b.color[0] = colors[i][0]; b.color[1] = colors[i][1]; b.color[2] = colors[i][2];

        px += b.mass * b.vel[0];
        pz += b.mass * b.vel[2];
        gBalls.push_back(b);
    }

    gBalls[0].vel[0] = -px / gBalls[0].mass;
    gBalls[0].vel[2] = -pz / gBalls[0].mass;
}

int FindHeaviestBall()
{
    int best = -1;
    for(int i = 0; i < (int)gBalls.size(); i++){
        if(best < 0 || gBalls[i].mass > gBalls[best].mass) best = i;
    }

    return best;
}

float RadiusFromMass(float mass)
{
    const float k           = 0.6f;
    const float exponent    = 0.2f;
    const float minRadius   = 0.15f;
    const float maxRadius   = 2.2f;

    if(mass < 0.0f) mass = 0.0f;

    float r = k * powf(mass, exponent);

    if(r < minRadius)   r = minRadius;
    if(r > maxRadius)   r = maxRadius;

    return r;
}

void ResolveCollision(int ball1, int ball2)
{
    if(ball1 < 0 || ball2 < 0 || ball1 >= (int)gBalls.size() || ball2 >= (int)gBalls.size() || ball1 == ball2)  return;

    float squaredRadius = powf(RadiusFromMass(gBalls[ball1].mass) + RadiusFromMass(gBalls[ball2].mass), 2);
    float distance = powf(sqrtf(powf(gBalls[ball1].pos[0] - gBalls[ball2].pos[0], 2) + powf(gBalls[ball1].pos[1] - gBalls[ball2].pos[1], 2) + powf(gBalls[ball2].pos[2] - gBalls[ball2].pos[2], 2)), 2);

    if(distance < squaredRadius){
        float newMass = gBalls[ball1].mass + gBalls[ball2].mass;
        if(newMass <= 0.0f) return;

        float newVelocity[3];
        float newPosition[3];

        for(int i = 0; i < 3; i++){
            newVelocity[i] = (gBalls[ball1].mass * gBalls[ball1].vel[i] + gBalls[ball2].mass * gBalls[ball2].vel[i]) / newMass;
            newPosition[i] = (gBalls[ball1].mass * gBalls[ball1].pos[i] + gBalls[ball2].mass * gBalls[ball2].pos[i]) / newMass;
        }

        if(gBalls[ball1].mass > gBalls[ball2].mass){
            gBalls[ball1].mass = newMass;

            for(int i = 0; i < 3; i++){
                gBalls[ball1].vel[i] = newVelocity[i];
                gBalls[ball1].pos[i] = newPosition[i];
            }
            gBalls[ball2].dead = true;

        } else{
            gBalls[ball2].mass = newMass;

            for(int i = 0; i < 3; i++){
                gBalls[ball2].vel[i] = newVelocity[i];
                gBalls[ball2].pos[i] = newPosition[i];
            }
            gBalls[ball1].dead = true;
        }
    }
}
