#pragma once

#include "ogldev_math_3d.h"
#include "ogldev_util.h"
#include <vector>
#include <math.h>

const float PI =    3.14150265358979;
#define MAX_LOD     2

struct MeshLOD
{
    GLuint  vao = 0;
    GLuint  vbo = 0;
    GLuint  ibo = 0;
    GLsizei indexCount = 0;
};

struct BallInstance
{
    float   pos[3];
    float   color[3];
    float   vel[3]              = { 0.0f, 0.0f, 0.0f };
    float   mass                = 1.0f;
    bool    dead                = false;
    float   collisionRadius     = 0.0f;
};

const int resolutions[3][2] = {
    { 16, 16 },
    { 12, 12 },
    { 8, 8}
};

class Ball
{
public:
    Ball(int stacks, int slices, float radius) :  m_radius(radius), m_indexCount(0) {};

    void CreateBall()
    {
        BallCreateVertexBuffer();
        BallCreateIndexBuffer();
    }

    void getRadius()
    {
        printf("Ball radius: %f", (float)m_radius);
    }

    void DrawBall(int tier) const
    {
        if(tier < 0 || tier > MAX_LOD)    tier = MAX_LOD;

        glBindVertexArray(m_lodTiers[tier].vao);
        glDrawElements(GL_TRIANGLES, m_lodTiers[tier].indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void DestroyBall()
    {
        for(int t = 0; t < 3; t++){
            glDeleteBuffers(1, &m_lodTiers[t].vbo);
            glDeleteBuffers(1, &m_lodTiers[t].ibo);
            glDeleteVertexArrays(1, &m_lodTiers[t].vao);
        }
    }

private:
    void BallCreateVertexBuffer()
    {
        const int rows = sizeof(resolutions) / sizeof(resolutions[0]);

        for(int t = 0; t < rows; t++){
            vector<Vector3f> Vertices;

            int stacks = resolutions[t][0];
            int slices = resolutions[t][1];

            for(int i = 0; i <= stacks; i++){
                float phi = PI * (float)i / (float)stacks;

                for(int j = 0; j <= slices; j++){
                    float theta = 2.0f * PI * (float)j / (float)slices;
                    float x = m_radius * sinf(phi) * cosf(theta);
                    float y = m_radius * cosf(phi);
                    float z = m_radius * sinf(phi) * sinf(theta);
                    Vertices.push_back(Vector3f(x, y, z));
                }
            }

            glGenVertexArrays(1, &m_lodTiers[t].vao);
            glBindVertexArray(m_lodTiers[t].vao);

            glGenBuffers(1, &m_lodTiers[t].vbo);
            glBindBuffer(GL_ARRAY_BUFFER, m_lodTiers[t].vbo);
            glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vector3f), Vertices.data(), GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void*)0);
            glBindVertexArray(0);
        }
    }

    void BallCreateIndexBuffer()
    {
        const int rows = sizeof(resolutions) / sizeof(resolutions[0]);

        for(int t = 0; t < rows; t++){
            vector<unsigned int> Indices;

            int stacks = resolutions[t][0];
            int slices = resolutions[t][1];

            for(int i = 0; i < stacks; i++){
                for(int j = 0; j < slices; j++){
                    unsigned int a = i * (slices + 1) + j;
                    unsigned int b = a + slices + 1;

                    Indices.push_back(a);
                    Indices.push_back(a + 1);

                    Indices.push_back(b);
                    Indices.push_back(a + 1);
                    Indices.push_back(b + 1);

                    Indices.push_back(b);
                }
            }

            m_indexCount = (GLsizei)Indices.size();
            m_lodTiers[t].indexCount = m_indexCount;

            glBindVertexArray(m_lodTiers[t].vao);
            glGenBuffers(1, &m_lodTiers[t].ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_lodTiers[t].ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), Indices.data(), GL_STATIC_DRAW);
        }
    }

    float     m_radius;
    GLsizei   m_indexCount;
    MeshLOD   m_lodTiers[3];
};
