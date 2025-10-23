#pragma once
#include "Renderer/Renderer.hpp"
#include "glm.hpp"

namespace Fermion
{
    class OpenGLRenderer : public IRenderer
    {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer();

        void drawRect(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color) override;
        void drawImage(const std::string &texturePath, const glm::vec2 &pos) override;

        void drawTriangle();

    private:
        unsigned int m_VAO = 0, m_VBO = 0;
        unsigned int m_shaderProgram = 0;

        void init();
        unsigned int createShaderProgram();
    };
}
