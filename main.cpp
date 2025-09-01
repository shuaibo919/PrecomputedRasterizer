#include "utils.h"
#include "rasterizer.h"
#include <chrono>

const int SCR_WIDTH = 1024;
const int SCR_HEIGHT = 640;
const int NUM_TRIANGLES = 24;

int main()
{
    GLFWwindow *window = nullptr;
    {
        if (GlfwGladInitialization(&window, SCR_WIDTH, SCR_HEIGHT, "Rasterizer") == -1)
        {
            return -1;
        }
        glfwSwapInterval(1);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    }

    std::vector<glm::vec3> randomTriangles;
    {
        // randomTriangles.reserve(NUM_TRIANGLES * 3);
        // for (int i = 0; i < NUM_TRIANGLES; ++i)
        // {
        //     for (int j = 0; j < 3; ++j)
        //     {
        //         float x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
        //         float y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
        //         randomTriangles.emplace_back(x, y, 0.0f);
        //     }
        // }
        randomTriangles = {
            { -0.5f, -0.5f, 0.0f },
            {  0.5f, -0.5f, 0.0f },
            {  0.0f,  0.5f, 0.0f }
        };
    }

    Rasterizer rasterizer(SCR_WIDTH, SCR_HEIGHT);
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Rasterization started." << std::endl;
        // Perform rasterization
        std::cout << "Rasterizing " << randomTriangles.size() / 3 << " triangles." << std::endl;
        rasterizer.RasterizePrototype3(randomTriangles);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end_time - start_time;
        std::cout << "Rasterization finished. Time elapsed: " << elapsed.count() << " ms" << std::endl;
    }

    {
        GLuint textureID = rasterizer.GetFrameBufferTexture();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLShader screenShader = GLShader("Shader/draw_screen");
        screenShader.Use();
        screenShader.SetUniform("screenTexture", 0);
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            RenderQuad();
            glfwSwapBuffers(window);
        }
        glfwTerminate();
    }

    return 0;
}
