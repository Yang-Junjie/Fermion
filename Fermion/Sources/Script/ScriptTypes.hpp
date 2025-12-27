#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <map>

namespace Fermion {
enum class ScriptFieldType {
    None = 0,
    Float,
    Double,
    Bool,
    Int,
    ULong,
    Vector2,
    Vector3,
    Vector4,
    Entity
};

struct ScriptField {
    std::string name;
    ScriptFieldType type;
};

struct ScriptFieldInstance {
    ScriptField field;

    ScriptFieldInstance() {
        m_buffer = std::monostate();
    }

    template <typename T>
    T getValue() const {
        try {
            return std::get<T>(m_buffer);
        } catch (const std::bad_variant_access &) {
            throw std::runtime_error("ScriptFieldInstance: 类型不匹配 (Type mismatch)");
        }
    }

    template <typename T>
    void setValue(T value) {
        m_buffer = value;
    }

private:
    std::variant<std::monostate, float, double, bool, int, uint64_t> m_buffer;
};

enum class ScriptHandleType {
    None,
    Object,
    Method
};

struct ScriptHandle {
    void *m_instance = nullptr;
    ScriptHandleType type = ScriptHandleType::None;

    bool isValid() const {
        return m_instance != nullptr && type != ScriptHandleType::None;
    }
};
} // namespace Fermion
