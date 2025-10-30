#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Fermion {

    class OpenGLShader {
    public:
        OpenGLShader() = default;
        OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
        ~OpenGLShader();

        // 禁止拷贝
        OpenGLShader(const OpenGLShader&) = delete;
        OpenGLShader& operator=(const OpenGLShader&) = delete;

        // 编译着色器
        void compile(const std::string& vertexSrc, const std::string& fragmentSrc);
        
        // 使用着色器程序
        void bind() const;
        void unbind() const;

        // 获取着色器程序ID
        uint32_t getRendererID() const { return m_RendererID; }

        // 设置uniform变量
        void setInt(const std::string& name, int value);
        void setFloat(const std::string& name, float value);
        void setFloat3(const std::string& name, float v0, float v1, float v2);
        void setFloat3(const std::string& name, const glm::vec3& value);
        void setFloat4(const std::string& name, float v0, float v1, float v2, float v3);
        void setFloat4(const std::string& name, const glm::vec4& value);
        void setMat4(const std::string& name, const glm::mat4& matrix);

    private:
        uint32_t m_RendererID = 0;
        mutable std::unordered_map<std::string, int> m_UniformLocationCache;

        int getUniformLocation(const std::string& name) const;
    };

}