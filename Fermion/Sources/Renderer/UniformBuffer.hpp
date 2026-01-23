#pragma once
#include "fmpch.hpp"

namespace Fermion
{
    // Abstract uniform buffer interface for platform-agnostic UBO management
    class UniformBuffer
    {
    public:
        virtual ~UniformBuffer() = default;

        // Bind the uniform buffer to its binding point
        virtual void bind() const = 0;

        // Unbind the uniform buffer
        virtual void unbind() const = 0;

        // Update buffer data (offset in bytes, size in bytes)
        virtual void setData(const void *data, uint32_t size, uint32_t offset = 0) = 0;

        // Get the buffer size in bytes
        virtual uint32_t getSize() const = 0;

        // Get the binding point for this UBO
        virtual uint32_t getBindingPoint() const = 0;

        // Factory method for creating platform-specific uniform buffers
        // bindingPoint: The UBO binding point (matches shader layout binding)
        // size: Buffer size in bytes
        static std::shared_ptr<UniformBuffer> create(uint32_t bindingPoint, uint32_t size);
    };
} // namespace Fermion
