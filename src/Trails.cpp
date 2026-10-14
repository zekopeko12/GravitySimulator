#include "Trails.h"
#include "Ball.h"
#include "Pipeline.h"
#include "Camera.h"
#include <GL/glew.h>
#include <vector>

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
        return;
    }

    static float timer = 0.0f;
    timer += dt;
    if(timer < kTrailInterval) return;

    timer = 0.0f;

    if(gTrails.size() != gBalls.size()){
        gTrails.resize(gBalls.size());
    }

    const float GM = GravitationalConstant * gBalls[0].mass;

    for(int i = 0; i < gBalls.size(); i++){
        vector<float>& t = gTrails[i];
        t.push_back(gBalls[i].pos[0]);
        t.push_back(gBalls[i].pos[1]);
        t.push_back(gBalls[i].pos[2]);

        size_t maxPoints = 20;
        if(i > 0){
            const float dx = gBalls[i].pos[0] - gBalls[0].pos[0];
            const float dy = gBalls[i].pos[1] - gBalls[0].pos[1];
            const float dz = gBalls[i].pos[2] - gBalls[0].pos[2];
            const float r  = sqrtf(dx * dx + dy * dy + dz * dz);
            const float period = 6.2831853f * r * sqrtf(r) / sqrtf(GM);

            maxPoints = (size_t)(kTrailFraction * period / kTrailInterval);
            if(maxPoints < 10)  maxPoints = 10;
            if(maxPoints > kMaxTrailPoints) maxPoints = kMaxTrailPoints;
        }

        while(t.size() > maxPoints * 3){
            t.erase(t.begin(), t.begin() + 3);
        }
    }
}

static void DrawTrails()
{
    Pipeline p;
    p.Scale(1.0f, 1.0f, 1.0f);
    p.WorldPos(0.0f, 0.0f, 0.0f);
    p.Rotate(0.0f, 0.0f, 0.0f);
    p.SetCamera(pGameCamera->GetPos(), pGameCamera->GetTarget(), pGameCamera->GetUp());
    p.SetPerspectiveProj(30.0f,
                         (float)glutGet(GLUT_WINDOW_WIDTH),
                         (float)glutGet(GLUT_WINDOW_HEIGHT) > 0 ? (float)glutGet(GLUT_WINDOW_HEIGHT) : 1.0f,
                         1.0f, 1000.0f);
    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());

    glBindVertexArray(gTrailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gTrailVBO);

    for(int i = 0; i < gTrails.size() && i < gBalls.size(); i++){
        const vector<float>& t = gTrails[i];
        if(t.size() < 6)    continue;

        glUniform3f(gColorLocation, gBalls[i].color[0], gBalls[i].color[1], gBalls[i].color[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, t.size() * sizeof(float), t.data());
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(t.size() / 3));
    }
}
