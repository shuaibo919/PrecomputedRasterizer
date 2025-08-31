#pragma once
#include "glad/glad.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <string>

class GLShader
{
private:
    std::string GLSL;
    GLuint mProgram;
    void AttachGLSL(std::string glsl_file_path, GLenum type);

public:
    GLShader(std::string glsl_file_path, bool load_geometry = false);
    GLuint GetShaderProgram();
    void SetUniform(const std::string &name, int value);
    void SetUniform(const std::string &name, float value);
    void SetUniform(const std::string &name, bool value);
    void SetUniform(const std::string &name, glm::mat4 mat4);
    void SetUniform(const std::string &name, glm::mat3 mat3);
    void SetUniform(const std::string &name, glm::vec3 vec3);
    void Use()
    {
        glUseProgram(mProgram);
    }
    ~GLShader();
};


int GlfwGladInitialization(GLFWwindow **window, int SRC_WIDTH, int SRC_HEIGHT, const char *title);
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
GLuint LoadTextureFromArray(const unsigned char *Array, int width, int height, GLint mode = GL_REPEAT, bool gamma = false);

void RenderQuad();