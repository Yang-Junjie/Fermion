#pragma once
#include "fmpch.hpp"

namespace Fermion {
enum class ShaderDataType {
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Int,
    Int2,
    Int3,
    Int4,
    Bool
};

struct BufferElement {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    size_t offset;
    bool normalized;
    uint32_t divisor = 0;

    BufferElement() = default;

    BufferElement(ShaderDataType type,
                  const std::string& name,
                  bool normalized = false,
                  uint32_t divisor = 0);

    uint32_t getComponentCount() const;
};

class BufferLayout {
public:
    BufferLayout() = default;

    BufferLayout(std::initializer_list<BufferElement> elements);

    uint32_t getStride() const;
    const std::vector<BufferElement>& getElements() const;

    std::vector<BufferElement>::iterator begin();
    std::vector<BufferElement>::iterator end();

    std::vector<BufferElement>::const_iterator begin() const;
    std::vector<BufferElement>::const_iterator end() const;

private:
    void calculateOffsetsAndStride();

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;
};

class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual void setData(const void* data, uint32_t size) = 0;

    virtual const BufferLayout& getLayout() const = 0;
    virtual void setLayout(const BufferLayout& layout) = 0;

    static std::shared_ptr<VertexBuffer> create(uint32_t size);
    static std::shared_ptr<VertexBuffer> create(float* vertices, uint32_t size);
};

// Currently only supports 32-bit index buffers
class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual uint32_t getCount() const = 0;

    static std::shared_ptr<IndexBuffer> create(uint32_t* indices, uint32_t count);
};
} // namespace Fermion
