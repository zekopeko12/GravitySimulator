#include "Trails.h"
#include "Ball.h"
#include "Pipeline.h"
#include "Camera.h"
#include <GL/glew.h>
#include <vector>

static vector<size_t> gTrailStart;

static void InitTrails()
{
    glGenVertexArrays(1, &gTrailVAO);
    glBindVertexArray(gTrailVAO);

    glGenBuffers(1, &gTrailVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTrailVBO);
    glBufferData(GL_ARRAY_BUFFER, kMaxTrailPoints * 3 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

static void RecordTrails(float dt)
{
    if(gBalls.empty()){
        gTrails.clear();
        gTrailStart.clear();
        return;
    }

    static float timer = 0.0f;
    timer += dt;
    if(timer < kTrailInterval) return;

    const float spacing = timer;
    timer = 0.0f;

    if(gTrails.size() != gBalls.size()){
        gTrails.resize(gBalls.size());
    }

    if(gTrailStart.size() != gBalls.size()){
        gTrailStart.resize(gBalls.size(), 0);
    }

    const float GM = GravitationalConstant * gBalls[0].mass;

    for(int i = 0; i < gBalls.size(); i++){
        if(gBalls[i].dead)  continue;

        vector<float>& t = gTrails[i];
        size_t& start = gTrailStart[i];

        if(t.empty())   start = 0;

        t.push_back(gBalls[i].pos[0]);
        t.push_back(gBalls[i].pos[1]);
        t.push_back(gBalls[i].pos[2]);

        size_t maxPoints = 10;
        if(i > 0){
            const float dx = gBalls[i].pos[0] - gBalls[0].pos[0];
            const float dy = gBalls[i].pos[1] - gBalls[0].pos[1];
            const float dz = gBalls[i].pos[2] - gBalls[0].pos[2];
            const float r  = sqrtf(dx * dx + dy * dy + dz * dz);
            const float period = 6.2831853f * r * sqrtf(r) / sqrtf(GM);

            maxPoints = (size_t)(kTrailFraction * period / spacing);
            if(maxPoints < 60)  maxPoints = 60;
            if(maxPoints > kMaxTrailPoints) maxPoints = kMaxTrailPoints;
        }

        const size_t count = t.size() / 3 - start;
        if(count > maxPoints)   start += count - maxPoints;

        if(start > 512){
            t.erase(t.begin(), t.begin() + start * 3);
            start = 0;
        }
    }
}

static void DrawTrails()
{
    float width  = (float)glutGet(GLUT_WINDOW_WIDTH);
    float height = (float)glutGet(GLUT_WINDOW_HEIGHT);
    if(height <= 0.0f)  height = 1.0f;

    Pipeline p;
    p.Scale(1.0f, 1.0f, 1.0f);
    p.WorldPos(0.0f, 0.0f, 0.0f);
    p.Rotate(0.0f, 0.0f, 0.0f);
    p.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    p.SetPerspectiveProj(gCurrentFOV, width, height, 1.0f, 1000.0f);
    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glBindVertexArray(gTrailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gTrailVBO);

    for(int i = 0; i < (int)gTrails.size() && i < (int)gBalls.size() && i < (int)gTrailStart.size(); i++){
        if(gBalls[i].dead)  continue;

        const vector<float>& t = gTrails[i];
        const size_t total = t.size() / 3;
        size_t first = gTrailStart[i];
        if(total < first + 2)   continue;

        size_t count = total - first;
        if(count > kMaxTrailPoints){
            first += count - kMaxTrailPoints;
            count = kMaxTrailPoints;
        }

        glUniform3f(gColorLocation, gBalls[i].color[0], gBalls[i].color[1], gBalls[i].color[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * 3 * sizeof(float), t.data() + first * 3);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)count);
    }
}
