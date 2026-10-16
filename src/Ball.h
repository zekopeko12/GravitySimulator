#pragma once

#include "ogldev_math_3d.h"
#include "ogldev_util.h"
#include <vector>
#include <math.h>

const float PI = 3.14150265358979;

struct BallInstance
{
    float pos[3];
    float color[3];
    float vel[3]            = { 0.0f, 0.0f, 0.0f };
    float mass              = 1.0f;
    bool dead               = false;
    float collisionRadius   = 0.0f;
};

class Ball
{
  public:
      Ball(int stacks, int slices, float radius)
        : m_stacks(stacks), m_slices(slices), m_radius(radius), m_VAO(0), m_VBO(0), m_IBO(0), m_indexCount(0) {};

      void CreateBall()
      {
          BallCreateVertexBuffer();
          BallCreateIndexBuffer();
      }


      void DrawBall() const
      {
          glBindVertexArray(m_VAO);
          glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
          glBindVertexArray(0);
      }

      void DestroyBall()
      {
          glDeleteBuffers(1, &m_VBO);
          glDeleteBuffers(1, &m_IBO);
          glDeleteVertexArrays(1, &m_VAO);
          m_VBO = m_IBO = m_VAO = 0;
      }

  private:
      void BallCreateVertexBuffer()
      {
          vector<Vector3f> Vertices;

          for(int i = 0; i <= m_stacks; i++){
              float phi = PI * (float)i / (float)m_stacks;
              for(int j = 0; j <= m_slices; j++){
                  float theta = 2.0f * PI * (float)j / (float)m_slices;
                  float x = m_radius * sinf(phi) * cosf(theta);
                  float y = m_radius * cosf(phi);
                  float z = m_radius * sinf(phi) * sinf(theta);
                  Vertices.push_back(Vector3f(x, y, z));
              }
          }

          glGenVertexArrays(1, &m_VAO);
          glBindVertexArray(m_VAO);

          glGenBuffers(1, &m_VBO);
          glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
          glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vector3f), Vertices.data(), GL_STATIC_DRAW);

          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void*)0);

          glBindVertexArray(0);
      }

      void BallCreateIndexBuffer()
      {
          vector<unsigned int> Indices;

          for(int i = 0; i < m_stacks; i++){
              for(int j = 0; j < m_slices; j++){
                  unsigned int a = i * (m_slices + 1) + j;
                  unsigned int b = a + m_slices + 1;

                  Indices.push_back(a);
                  Indices.push_back(a + 1);
                  Indices.push_back(b);

                  Indices.push_back(a + 1);
                  Indices.push_back(b + 1);
                  Indices.push_back(b);
              }
          }

          m_indexCount = (GLsizei)Indices.size();

          glBindVertexArray(m_VAO);

          glGenBuffers(1, &m_IBO);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), Indices.data(), GL_STATIC_DRAW);
      }

      int       m_stacks;
      int       m_slices;
      float     m_radius;

      GLuint    m_VAO;
      GLuint    m_VBO;
      GLuint    m_IBO;
      GLsizei   m_indexCount;
};
