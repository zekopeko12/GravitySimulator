#pragma once
#include <GL/glew.h>

GLuint CompileShader(GLenum type, const char* text);
void CompileShaders(GLuint& gShaderProgram, const char* VertexShaderText,
                    const char* FragmentShaderText, GLint& gWVPLocation, GLint& gColorLocation);
