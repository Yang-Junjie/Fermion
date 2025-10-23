#include "OpenGLRenderer.hpp"
#include "Core/Log.hpp"
#include <glad/glad.h>
#include <string>

namespace Fermion
{
    OpenGLRenderer::OpenGLRenderer()
    {
        if (!gladLoadGL())
        {
            Log::Error("Failed to initialize GLAD!");
            return;
        }

        Log::Info("OpenGL initialized successfully!");
        init();
    }

    OpenGLRenderer::~OpenGLRenderer()
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteProgram(m_shaderProgram);
    }

    void OpenGLRenderer::init()
    {
        float vertices[] = {
            // positions        // colors
            -0.5f, -0.5f, 0.0f,  1.f, 0.f, 0.f,
             0.5f, -0.5f, 0.0f,  0.f, 1.f, 0.f,
             0.0f,  0.5f, 0.0f,  0.f, 0.f, 1.f,
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        m_shaderProgram = createShaderProgram();
    }

    unsigned int OpenGLRenderer::createShaderProgram()
    {
        const char* vertexShaderSrc = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aColor;
            out vec3 vColor;
            void main()
            {
                gl_Position = vec4(aPos, 1.0);
                vColor = aColor;
            }
        )";

        const char* fragmentShaderSrc = R"(
            #version 330 core
            in vec3 vColor;
            out vec4 FragColor;
            void main()
            {
                FragColor = vec4(vColor, 1.0);
            }
        )";

        auto compileShader = [](GLenum type, const char* src) -> unsigned int {
            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);
            int success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char info[512];
                glGetShaderInfoLog(shader, 512, nullptr, info);
                Log::Error(std::string("Shader compilation failed: ") + info);
            }
            return shader;
        };

        unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
        unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

        unsigned int program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    void OpenGLRenderer::drawTriangle()
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(m_shaderProgram);
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void OpenGLRenderer::drawRect(const glm::vec2 &, const glm::vec2 &, const glm::vec4 &) {}
    void OpenGLRenderer::drawImage(const std::string &, const glm::vec2 &) {}
}
