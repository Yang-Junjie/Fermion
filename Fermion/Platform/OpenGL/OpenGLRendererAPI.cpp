#include "OpenGLRendererAPI.hpp"
#include <glad/glad.h>
namespace Fermion
{
    void OpenGLRendererAPI::init()
    {
        FM_PROFILE_FUNCTION();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
    }

    void OpenGLRendererAPI::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }
    void OpenGLRendererAPI::setClearColor(const glm::vec4 &color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }
    void OpenGLRendererAPI::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount)
    {
        vertexArray->bind();
        uint32_t count = indexCount ? indexCount : vertexArray->getIndexBuffer()->getCount();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenGLRendererAPI::drawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t vertexCount)
    {
        vertexArray->bind();
        glDrawArrays(GL_LINES, 0, vertexCount);
    }

    void OpenGLRendererAPI::setLineWidth(float width)
    {
        glLineWidth(width);
    }
}