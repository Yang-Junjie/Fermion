#include "OpenGLShader.hpp"
#include "Core/Log.hpp"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <fstream>
#include <sstream>
#include "glad/glad.h"
namespace Fermion
{

    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc)
    {
        compile(vertexSrc, fragmentSrc);
    }

    OpenGLShader::~OpenGLShader()
    {
        if (m_RendererID != 0)
        {
            glDeleteProgram(m_RendererID);
        }
    }

    void OpenGLShader::compile(const std::string &vertexSrc, const std::string &fragmentSrc)
    {
        // 创建着色器
        uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char *vertexSource = vertexSrc.c_str();
        glShaderSource(vertexShader, 1, &vertexSource, nullptr);
        glCompileShader(vertexShader);

        // 检查顶点着色器编译错误
        int isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            int maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertexShader);

            Log::Error("Vertex Shader compilation error");
            return;
        }

        // 创建片段着色器
        uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fragmentSource = fragmentSrc.c_str();
        glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
        glCompileShader(fragmentShader);

        // 检查片段着色器编译错误
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            int maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            Log::Error("Fragment Shader compilation error");
            return;
        }

        // 创建着色器程序并链接
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);

        // 检查链接错误
        int isLinked = 0;
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            int maxLength = 0;
            glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<char> infoLog(maxLength);
            glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(m_RendererID);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            Log::Error("Shader program linking error");
            return;
        }

        // 链接成功后删除着色器对象
        glDetachShader(m_RendererID, vertexShader);
        glDetachShader(m_RendererID, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        Log::Trace("Shader compiled and linked successfully");
    }

    void OpenGLShader::bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::unbind() const
    {
        glUseProgram(0);
    }

    int OpenGLShader::getUniformLocation(const std::string &name) const
    {
        // 先在缓存中查找
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        {
            return m_UniformLocationCache[name];
        }

        // 如果缓存中没有，则查询并缓存
        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
        {
            Log::Warn("Uniform '{0}' not found in shader");
        }

        m_UniformLocationCache[name] = location;
        return location;
    }

    void OpenGLShader::setInt(const std::string &name, int value)
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void OpenGLShader::setFloat(const std::string &name, float value)
    {
        glUniform1f(getUniformLocation(name), value);
    }

    void OpenGLShader::setFloat3(const std::string &name, float v0, float v1, float v2)
    {
        glUniform3f(getUniformLocation(name), v0, v1, v2);
    }

    void OpenGLShader::setFloat4(const std::string &name, float v0, float v1, float v2, float v3)
    {
        glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
    }

    void OpenGLShader::setMat4(const std::string &name, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

}