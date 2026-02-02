#pragma once
#include <vector>
#include <cstdint>
#include <cstring>

namespace Fermion
{
    template <typename T>
    class BatchBuffer
    {
    public:
        explicit BatchBuffer(uint32_t maxElements)
            : m_MaxElements(maxElements), m_CurrentIndex(0)
        {
            m_Buffer.resize(maxElements);
        }

        ~BatchBuffer() = default;

        BatchBuffer(const BatchBuffer &) = delete;
        BatchBuffer &operator=(const BatchBuffer &) = delete;
        BatchBuffer(BatchBuffer &&) noexcept = default;
        BatchBuffer &operator=(BatchBuffer &&) noexcept = default;

        void reset()
        {
            m_CurrentIndex = 0;
        }

        bool write(const T &element)
        {
            if (m_CurrentIndex >= m_MaxElements)
                return false;

            m_Buffer[m_CurrentIndex++] = element;
            return true;
        }

        T *current()
        {
            if (m_CurrentIndex >= m_MaxElements)
                return nullptr;
            return &m_Buffer[m_CurrentIndex];
        }

        void advance()
        {
            if (m_CurrentIndex < m_MaxElements)
                ++m_CurrentIndex;
        }

        void advance(uint32_t count)
        {
            m_CurrentIndex = std::min(m_CurrentIndex + count, m_MaxElements);
        }

        const T *data() const { return m_Buffer.data(); }
        T *data() { return m_Buffer.data(); }
        uint32_t getDataSize() const
        {
            return m_CurrentIndex * sizeof(T);
        }

        uint32_t getCount() const { return m_CurrentIndex; }
        uint32_t getCapacity() const { return m_MaxElements; }
        bool isEmpty() const { return m_CurrentIndex == 0; }

        bool isFull() const { return m_CurrentIndex >= m_MaxElements; }
        bool hasSpace(uint32_t count = 1) const
        {
            return (m_CurrentIndex + count) <= m_MaxElements;
        }

    private:
        std::vector<T> m_Buffer;
        uint32_t m_MaxElements;
        uint32_t m_CurrentIndex;
    };

} // namespace Fermion
