#include "Gravity.h"
#include "Ball.h"
#include "CBFunctions.h"
#include "Trails.h"

static float gMoonAngle = 45.0f * PI / 180.0f;

void StepGravity(float dt, float maxStep)
{
    int substeps = (int)ceilf(dt / maxStep);
    if(substeps < 1) substeps = 1;

    const float h = dt / substeps;
    const float half_h = 0.5f * h;
    const float h2_half = 0.5f * h * h;
    const float softening = 0.0000001f;

    static vector<float> acc, acc_new;

    int n = (int)gBalls.size();
    acc.assign(n * 3, 0.0f);
    acc_new.assign(n * 3, 0.0f);

    auto computeAccelerations = [&](vector<float>& target_acc){
        fill(target_acc.begin(), target_acc.end(), 0.0f);

        for(int i = 0; i < n; i++){
            if(gBalls[i].dead) continue;
            for(int j = i + 1; j < n; j++){
                if(gBalls[j].dead) continue;

                BallInstance& a = gBalls[i];
                BallInstance& b = gBalls[j];

                const float dx = b.pos[0] - a.pos[0];
                const float dy = b.pos[1] - a.pos[1];
                const float dz = b.pos[2] - a.pos[2];

                const float d2 = dx * dx + dy * dy + dz * dz + softening;
                const float inv3 = 1.0f / (d2 * sqrtf(d2));

                const float kA = GravitationalConstant * b.mass * inv3;
                const float kB = GravitationalConstant * a.mass * inv3;

                target_acc[i * 3 + 0] += kA * dx;
                target_acc[i * 3 + 1] += kA * dy;
                target_acc[i * 3 + 2] += kA * dz;

                target_acc[j * 3 + 0] -= kB * dx;
                target_acc[j * 3 + 1] -= kB * dy;
                target_acc[j * 3 + 2] -= kB * dz;
            }
        }
    };

    computeAccelerations(acc);

    for(int s = 0; s < substeps; s++){
        for(int i = 0; i < n; i++){
            BallInstance& b = gBalls[i];
            if(b.dead)  continue;

            for(int c = 0; c < 3; c++){
                float a_val = acc[i * 3 + c];
                b.pos[c] += b.vel[c] * h + a_val * h2_half;
                b.vel[c] += a_val * half_h;
            }
        }

        computeAccelerations(acc_new);

        for(int i = 0; i < n; i++){
            BallInstance& b = gBalls[i];
            if(b.dead)  continue;

            for(int c = 0; c < 3; c++){
                b.vel[c] += acc_new[i * 3 + c] * half_h;
            }
        }

        if(!gAsteroids.empty() && !gBalls.empty() && !gBalls[0].dead) {
            BallInstance& sun = gBalls[0];

            for(auto& ast : gAsteroids){
                const float dx = sun.pos[0] - ast.pos[0];
                const float dy = sun.pos[1] - ast.pos[1];
                const float dz = sun.pos[2] - ast.pos[2];

                const float d2 = dx * dx + dy * dy + dz * dz + softening;
                const float inv3 = 1.0f / (d2 * sqrtf(d2));

                const float k = GravitationalConstant * sun.mass * inv3;

                ast.vel[0] += k * dx * h;
                ast.vel[1] += k * dy * h;
                ast.vel[2] += k * dz * h;

                ast.pos[0] += ast.vel[0] * h;
                ast.pos[1] += ast.vel[1] * h;
                ast.pos[2] += ast.vel[2] * h;
            }
        }

        bool merged = false;
        for(int i = 0; i < (int)gBalls.size(); i++){
            if(gBalls[i].dead)  continue;

            for(int j = i + 1; j < (int)gBalls.size(); j++){
                if(gBalls[j].dead)  continue;

                ResolveCollision(i, j);

                if(gBalls[i].dead || gBalls[j].dead)    merged = true;
                if(gBalls[i].dead)  break;
            }
        }

        if(gBalls.size() > 3 && !gBalls[3].dead && gMoonInitialized)
        {
            BallInstance& earth = gBalls[3];

            const float moonDistance = 0.6f;
            const float moonSpeed = 2.0f;

            gMoonAngle += moonSpeed * h;

            gMoon.pos[0] = earth.pos[0] + moonDistance * cosf(gMoonAngle);
            gMoon.pos[1] = earth.pos[1];
            gMoon.pos[2] = earth.pos[2] + moonDistance * sinf(gMoonAngle);

            gMoon.vel[0] = earth.vel[0];
            gMoon.vel[1] = earth.vel[1];
            gMoon.vel[2] = earth.vel[2];
        }

        RecordTrails(h);

        if(merged){
            n = (int)gBalls.size();
            computeAccelerations(acc);
        } else        acc.swap(acc_new);
    }
}

void SolarSystemDemo()
{
    gBalls.clear();
    gAsteroids.clear();
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
        if(i == 4)  AsteroidPlaneSetup(sun, px, pz);

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
        b.collisionRadius = (i==2) ? 0.008f : RadiusFromMass(b.mass);

        px += b.mass * b.vel[0];
        pz += b.mass * b.vel[2];
        gBalls.push_back(b);

        if(i == 2){
            BallInstance& earth = gBalls.back();

            const float moonDistance = 0.6f;
            const float moonMass = 0.0123f * kPlanetMassScale;

            gMoon.pos[0] = earth.pos[0] + moonDistance * cosf(gMoonAngle);
            gMoon.pos[1] = earth.pos[1];
            gMoon.pos[2] = earth.pos[2] + moonDistance * sinf(gMoonAngle);

            const float moonSpeed = sqrtf(GravitationalConstant * earth.mass / moonDistance);
            gMoon.vel[0] = earth.vel[0] - moonSpeed * sinf(gMoonAngle);
            gMoon.vel[1] = earth.vel[1];
            gMoon.vel[2] = earth.vel[2] + moonSpeed * cosf(gMoonAngle);

            gMoon.mass = moonMass;
            gMoon.dead = false;
            gMoon.color[0] = 0.72f; gMoon.color[1] = 0.72f; gMoon.color[2] = 0.76f;
            gMoon.collisionRadius = 0.002f;
            gMoonInitialized = true;
        }
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
    const float minRadius   = 0.0000001f;
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

    BallInstance& a = gBalls[ball1];
    BallInstance& b = gBalls[ball2];

    float dx = b.pos[0] - a.pos[0];
    float dy = b.pos[1] - a.pos[1];
    float dz = b.pos[2] - a.pos[2];

    float distanceSquared = dx * dx + dy * dy + dz * dz;
    float radiusA = (a.collisionRadius > 0.0f) ? a.collisionRadius : RadiusFromMass(a.mass);
    float radiusB = (b.collisionRadius > 0.0f) ? b.collisionRadius : RadiusFromMass(b.mass);
    float radiusSum = radiusA + radiusB;

    if(distanceSquared >= radiusSum * radiusSum)    return;

    float totalMass = a.mass + b.mass;
    if(totalMass <= 0.0f)   return;

    float newVelocity[3];
    float newPosition[3];

    for(int c = 0; c < 3; c++){
        newVelocity[c] = (a.mass * a.vel[c] + b.mass * b.vel[c]) / totalMass;
        newPosition[c] = (a.mass * a.pos[c] + b.mass * b.pos[c]) / totalMass;
    }

    int survivor = (a.mass >= b.mass) ? ball1 : ball2;
    int removed = (survivor == ball1) ? ball2 : ball1;

    gBalls[survivor].mass = totalMass;

    for(int c = 0; c < 3; c++){
        gBalls[survivor].vel[c] = newVelocity[c];
        gBalls[survivor].pos[c] = newPosition[c];
    }

    gBalls[removed].dead = true;
}

void AsteroidPlaneSetup(BallInstance sun, float& px, float& pz)
{
    const float center[3] = { 0.0f, 0.0f, 0.8f };

    for(int i = 0; i < 2000; i++){
        BallInstance a;
        float randomDistance = 16.0f + (28.0f - 16.0f) * ((float)rand() / RAND_MAX);
        float randomMass = 0.001f + (0.01 - 0.001) * ((float)rand() / RAND_MAX);
        float randomAngle = ((float)rand() / RAND_MAX) * 6.2831853f;
        const float v = sqrtf(GravitationalConstant * sun.mass / randomDistance);

        a.pos[0] = center[0] + randomDistance * cosf(randomAngle);
        a.pos[1] = center[1];
        a.pos[2] = center[2] + randomDistance * sinf(randomAngle);

        a.vel[0] = -v * sinf(randomAngle);
        a.vel[1] = 0.0f;
        a.vel[2] = v * cosf(randomAngle);

        a.mass = randomMass * kPlanetMassScale;
        a.color[0] = 0.47f; a.color[1] = 0.47f; a.color[2] = 0.47f;

        px += a.mass * a.vel[0];
        pz += a.mass * a.vel[2];
        gAsteroids.push_back(a);
    }

    // Adding some outliers under and over the min/max distance for a more realistic look
    for(int i = 0; i < 50; i++){
        BallInstance a;
        float randomDistance = 14.0f + (16.0f - 14.0f) * ((float)rand() / RAND_MAX);
        float randomMass = 0.001f + (0.01 - 0.001) * ((float)rand() / RAND_MAX);
        float randomAngle = ((float)rand() / RAND_MAX) * 6.2831853f;
        const float v = sqrtf(GravitationalConstant * sun.mass / randomDistance);

        a.pos[0] = center[0] + randomDistance * cosf(randomAngle);
        a.pos[1] = center[1];
        a.pos[2] = center[2] + randomDistance * sinf(randomAngle);

        a.vel[0] = -v * sinf(randomAngle);
        a.vel[1] = 0.0f;
        a.vel[2] = v * cosf(randomAngle);

        a.mass = randomMass * kPlanetMassScale;
        a.color[0] = 0.47f; a.color[1] = 0.47f; a.color[2] = 0.47f;

        px += a.mass * a.vel[0];
        pz += a.mass * a.vel[2];
        gAsteroids.push_back(a);
    }

    for(int i = 0; i < 200; i++){
        BallInstance a;
        float randomDistance = 28.0f + (38.0f - 28.0f) * ((float)rand() / RAND_MAX);
        float randomMass = 0.001f + (0.01 - 0.001) * ((float)rand() / RAND_MAX);
        float randomAngle = ((float)rand() / RAND_MAX) * 6.2831853f;
        const float v = sqrtf(GravitationalConstant * sun.mass / randomDistance);

        a.pos[0] = center[0] + randomDistance * cosf(randomAngle);
        a.pos[1] = center[1];
        a.pos[2] = center[2] + randomDistance * sinf(randomAngle);

        a.vel[0] = -v * sinf(randomAngle);
        a.vel[1] = 0.0f;
        a.vel[2] = v * cosf(randomAngle);

        a.mass = randomMass * kPlanetMassScale;
        a.color[0] = 0.47f; a.color[1] = 0.47f; a.color[2] = 0.47f;

        px += a.mass * a.vel[0];
        pz += a.mass * a.vel[2];
        gAsteroids.push_back(a);
    }
}
