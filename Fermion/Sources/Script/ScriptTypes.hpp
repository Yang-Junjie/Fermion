#pragma once

#include <string>
#include <variant>
#include <stdexcept>
#include <map>

namespace Fermion
{
    // 支持的脚本字段类型枚举
    enum class ScriptFieldType
    {
        None = 0,
        Float,
        Double,
        Bool,
        Int,
        Vector2,
        Vector3,
        Vector4,
        Entity
    };

    // 脚本字段定义结构体
    struct ScriptField
    {
        std::string name;     // 字段名称
        ScriptFieldType type; // 字段类型
    };

    // 脚本字段实例，用于存储字段的具体值
    struct ScriptFieldInstance
    {
        ScriptField field; // 字段定义

        ScriptFieldInstance() { m_buffer = std::monostate(); }

        // 获取字段值
        template <typename T>
        T getValue() const
        {
            try
            {
                return std::get<T>(m_buffer);
            }
            catch (const std::bad_variant_access &)
            {
                throw std::runtime_error("ScriptFieldInstance: 类型不匹配 (Type mismatch)");
            }
        }

        // 设置字段值
        template <typename T>
        void setValue(T value)
        {
            m_buffer = value;
        }

    private:
        // 使用 variant 存储不同类型的值
        std::variant<std::monostate, float, double, bool, int> m_buffer;
    };

    // 脚本句柄类型枚举
    enum class ScriptHandleType
    {
        None,
        Object,
        Method
    };

    // 脚本句柄，用于引用脚本中的对象或方法
    struct ScriptHandle
    {
        void *m_instance = nullptr;                     // 原始指针
        ScriptHandleType type = ScriptHandleType::None; // 句柄类型

        // 检查句柄是否有效
        bool isValid() const { return m_instance != nullptr && type != ScriptHandleType::None; }
    };
}
