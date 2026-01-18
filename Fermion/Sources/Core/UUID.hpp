#pragma once
#include <cstdint>
#include <functional>
#include <string>
namespace Fermion
{

    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID &) = default;

        operator uint64_t() const
        {
            return m_UUID;
        }
        bool operator==(const UUID &other) const
        {
            return m_UUID == other.m_UUID;
        }
        std::string toString() const
        {
            return std::to_string(m_UUID);
        }

        bool isValid() const
        {
            return m_UUID != 0;
        }

    private:
        uint64_t m_UUID;
    };

} // namespace Fermion

namespace std
{
    template <typename T>
    struct hash;

    template <>
    struct hash<Fermion::UUID>
    {
        std::size_t operator()(const Fermion::UUID &uuid) const
        {
            return (uint64_t)uuid;
        }
    };

} // namespace std
