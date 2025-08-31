#include "utils.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

GLShader::GLShader(std::string glsl_file_path, bool load_geometry)
{
    mProgram = glCreateProgram();
    GLSL = glsl_file_path;
    AttachGLSL(GLSL + ".vs", GL_VERTEX_SHADER);
    AttachGLSL(GLSL + ".fs", GL_FRAGMENT_SHADER);
    if (load_geometry)
        AttachGLSL(GLSL + ".gs", GL_GEOMETRY_SHADER);
    glUseProgram(mProgram);
}

void GLShader::AttachGLSL(std::string glsl_file_path, GLenum type)
{
    std::string glslCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        auto glslPath = glsl_file_path;
        shaderFile.open(glslPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        glslCode = shaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ  PATH:" << glsl_file_path << std::endl;
    }
    const char *shaderCode = glslCode.c_str();
    GLuint tmpShader = glCreateShader(type);
    glShaderSource(tmpShader, 1, &shaderCode, NULL);
    glCompileShader(tmpShader);
    int success;
    char infoLog[512];
    glGetShaderiv(tmpShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(tmpShader, 512, NULL, infoLog);
        if(type == GL_VERTEX_SHADER)
        {
            
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
        else if(type == GL_FRAGMENT_SHADER)
        {
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
        else if(type == GL_GEOMETRY_SHADER)
        {
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        }
    }
    glAttachShader(mProgram, tmpShader);
    glLinkProgram(mProgram);
    glDeleteShader(tmpShader);
}

GLuint GLShader::GetShaderProgram()
{
    return mProgram;
}

void GLShader::SetUniform(const std::string &name, int value)
{
    glUseProgram(mProgram);
    glUniform1i(glGetUniformLocation(mProgram, name.c_str()), value);
}

void GLShader::SetUniform(const std::string &name, float value)
{
    glUseProgram(mProgram);
    glUniform1f(glGetUniformLocation(mProgram, name.c_str()), value);
}

void GLShader::SetUniform(const std::string &name, bool value)
{
    glUseProgram(mProgram);
    glUniform1i(glGetUniformLocation(mProgram, name.c_str()), (int)value);
}

void GLShader::SetUniform(const std::string &name, glm::mat4 mat4)
{
    glUseProgram(mProgram);
    glUniformMatrix4fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
}

void GLShader::SetUniform(const std::string &name, glm::mat3 mat3)
{
    glUseProgram(mProgram);
    glUniformMatrix3fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat3));
}

void GLShader::SetUniform(const std::string &name, glm::vec3 vec3)
{
    glUseProgram(mProgram);
    glUniform3fv(glGetUniformLocation(mProgram, name.c_str()), 1, glm::value_ptr(vec3));
}

GLShader::~GLShader()
{
    glDeleteProgram(mProgram);
}

GLuint LoadTextureFromArray(const unsigned char *Array, int width, int height, GLint mode, bool gamma)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    if (Array)
    {

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Array);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }

    return textureID;
}


int GlfwGladInitialization(GLFWwindow **window, int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(*window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return 0;
}

void FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void RenderQuad()
{
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}