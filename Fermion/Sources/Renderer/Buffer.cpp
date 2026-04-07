#include "OpenGLBuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Renderers/Renderer.hpp"

namespace Fermion {
namespace {

uint32_t ShaderDataTypeSize(const ShaderDataType type)
{
    switch (type) {
        case ShaderDataType::Float:
            return 4;
        case ShaderDataType::Float2:
            return 4 * 2;
        case ShaderDataType::Float3:
            return 4 * 3;
        case ShaderDataType::Float4:
            return 4 * 4;
        case ShaderDataType::Mat3:
            return 4 * 3 * 3;
        case ShaderDataType::Mat4:
            return 4 * 4 * 4;
        case ShaderDataType::Int:
            return 4;
        case ShaderDataType::Int2:
            return 4 * 2;
        case ShaderDataType::Int3:
            return 4 * 3;
        case ShaderDataType::Int4:
            return 4 * 4;
        case ShaderDataType::Bool:
            return 1;
        default:
            return 0;
    }
}

} // namespace

BufferElement::BufferElement(ShaderDataType type,
                             const std::string& name,
                             bool normalized,
                             uint32_t divisor)
    : name(name),
      type(type),
      size(ShaderDataTypeSize(type)),
      offset(0),
      normalized(normalized),
      divisor(divisor)
{}

uint32_t BufferElement::getComponentCount() const
{
    switch (type) {
        case ShaderDataType::Float:
            return 1;
        case ShaderDataType::Float2:
            return 2;
        case ShaderDataType::Float3:
            return 3;
        case ShaderDataType::Float4:
            return 4;
        case ShaderDataType::Mat3:
            return 3; // 3* float3
        case ShaderDataType::Mat4:
            return 4; // 4* float4
        case ShaderDataType::Int:
            return 1;
        case ShaderDataType::Int2:
            return 2;
        case ShaderDataType::Int3:
            return 3;
        case ShaderDataType::Int4:
            return 4;
        case ShaderDataType::Bool:
            return 1;
    }

    return 0;
}

BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements)
    : m_elements(elements)
{
    calculateOffsetsAndStride();
}

uint32_t BufferLayout::getStride() const
{
    return m_stride;
}

const std::vector<BufferElement>& BufferLayout::getElements() const
{
    return m_elements;
}

std::vector<BufferElement>::iterator BufferLayout::begin()
{
    return m_elements.begin();
}

std::vector<BufferElement>::iterator BufferLayout::end()
{
    return m_elements.end();
}

std::vector<BufferElement>::const_iterator BufferLayout::begin() const
{
    return m_elements.begin();
}

std::vector<BufferElement>::const_iterator BufferLayout::end() const
{
    return m_elements.end();
}

void BufferLayout::calculateOffsetsAndStride()
{
    size_t offset = 0;
    m_stride = 0;
    for (auto& element : m_elements) {
        element.offset = offset;
        offset += element.size;
        m_stride += element.size;
    }
}

std::shared_ptr<VertexBuffer> VertexBuffer::create(uint32_t size)
{
    switch (Renderer::getAPI()) {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(size);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan vertex buffer creation is not implemented yet.");
            return nullptr;
    }

    return nullptr;
}

std::shared_ptr<VertexBuffer> VertexBuffer::create(float* vertices, uint32_t size)
{
    switch (Renderer::getAPI()) {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan vertex buffer creation is not implemented yet.");
            return nullptr;
    }

    return nullptr;
}

std::shared_ptr<IndexBuffer> IndexBuffer::create(uint32_t* indices, uint32_t size)
{
    switch (Renderer::getAPI()) {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLIndexBuffer>(indices, size);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan index buffer creation is not implemented yet.");
            return nullptr;
    }

    return nullptr;
}

} // namespace Fermion
