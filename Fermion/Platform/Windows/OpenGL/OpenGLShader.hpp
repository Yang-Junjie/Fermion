#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Renderer/Shader.hpp"

namespace Fermion {

class OpenGLShader : public Shader {
public:
    OpenGLShader() = default;
    OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
    OpenGLShader(const std::string &filepath);
    virtual ~OpenGLShader();

    // 禁止拷贝
    OpenGLShader(const OpenGLShader &) = delete;
    OpenGLShader &operator=(const OpenGLShader &) = delete;

    // 使用着色器程序
    virtual void bind() const override;
    virtual void unbind() const override;

    // 获取着色器程序ID
    uint32_t getRendererID() const {
        return m_rendererID;
    }

    virtual void setInt(const std::string &name, int value) override;
    virtual void setIntArray(const std::string &name, int *values, uint32_t count) override;
    virtual void setBool(const std::string &name, bool value) override;
    virtual void setFloat(const std::string &name, float value) override;
    virtual void setFloat3(const std::string &name, float v0, float v1, float v2) override;
    virtual void setFloat4(const std::string &name, float v0, float v1, float v2, float v3) override;
    virtual void setFloat3(const std::string &name, const glm::vec3 &value) override;
    virtual void setFloat4(const std::string &name, const glm::vec4 &value) override;
    virtual void setMat4(const std::string &name, const glm::mat4 &matrix) override;

    // 设置uniform变量
    void uploadInt(const std::string &name, int value);
    void uploadIntArray(const std::string &name, int *values, uint32_t count);
    void uploadBool(const std::string &name, bool value);
    void uploadFloat(const std::string &name, float value);
    void uploadFloat3(const std::string &name, float v0, float v1, float v2);
    void uploadFloat3(const std::string &name, const glm::vec3 &value);
    void uploadFloat4(const std::string &name, float v0, float v1, float v2, float v3);
    void uploadFloat4(const std::string &name, const glm::vec4 &value);
    void uploadMat4(const std::string &name, const glm::mat4 &matrix);

    virtual const std::string &getName() const override {
        return m_name;
    }

private:
    int getUniformLocation(const std::string &name) const;
    void compile(const std::string &vertexSrc, const std::string &fragmentSrc);
    std::string readFile(const std::string &filepath);
    std::unordered_map<uint32_t, std::string> preProcess(const std::string &source);

private:
    uint32_t m_rendererID;
    std::string m_name;
    std::string m_filePathl;
    mutable std::unordered_map<std::string, int> m_UniformLocationCache;
};

} // namespace Fermion