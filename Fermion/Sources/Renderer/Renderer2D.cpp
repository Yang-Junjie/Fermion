#include "fmpch.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"

#include "glm/gtc/matrix_transform.hpp"
namespace Fermion
{
    struct Renderer2DData
    {
        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<Shader> FlatColorShader;
        std::shared_ptr<Shader> TextureShader;
    };

    static Renderer2DData s_Data;

    void Renderer2D::init()
    {
        s_Data.QuadVertexArray = VertexArray::create();

        float squareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};
        std::shared_ptr<VertexBuffer> squareVB = VertexBuffer::create(squareVertices, sizeof(squareVertices));
        squareVB->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
        });
        s_Data.QuadVertexArray->addVertexBuffer(squareVB);
        uint32_t indices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<IndexBuffer> squareIB = IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t));
        s_Data.QuadVertexArray->setIndexBuffer(squareIB);
        s_Data.FlatColorShader = Shader::create("../game/assets/shaders/FlatColor.glsl");
        s_Data.TextureShader = Shader::create("../game/assets/shaders/Texture.glsl");
        s_Data.TextureShader->bind();
        s_Data.TextureShader->setInt("u_Texture", 0);
    }

    void Renderer2D::shutdown()
    {
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera)
    {
        s_Data.FlatColorShader->bind();
        s_Data.FlatColorShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
        s_Data.TextureShader->bind();
        s_Data.TextureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
    }

    void Renderer2D::endScene()
    {
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        drawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        s_Data.FlatColorShader->bind();
        s_Data.FlatColorShader->setFloat4("u_Color", color);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data.FlatColorShader->setMat4("u_Transform", transform);
        s_Data.QuadVertexArray->bind();
        RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture)
    {
        drawQuad(glm::vec3(position, 0.0f), size, texture);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture)
    {
        s_Data.TextureShader->bind();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data.FlatColorShader->setMat4("u_Transform", transform);

        texture->bind();

        s_Data.QuadVertexArray->bind();
        RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }
}
