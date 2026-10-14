#include "Shader.h"
#include <stdio.h>

GLuint CompileShader(GLenum type, const char* text)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &text, NULL);
    glCompileShader(shader);

    return shader;
}

void CompileShaders(GLuint& gShaderProgram, const char* VertexShaderText, const char* FragmentShaderText, GLint& gWVPLocation, GLint& gColorLocation)
{
    gShaderProgram = glCreateProgram();
    GLuint vs = CompileShader(GL_VERTEX_SHADER, VertexShaderText);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShaderText);

    glAttachShader(gShaderProgram, vs);
    glAttachShader(gShaderProgram, fs);
    glLinkProgram(gShaderProgram);

    gWVPLocation = glGetUniformLocation(gShaderProgram, "gWVP");
    gColorLocation = glGetUniformLocation(gShaderProgram, "ballColor");
    glUseProgram(gShaderProgram);
}
