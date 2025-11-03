#include "OpenGLShader.hpp"
#include "Core/Log.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

namespace Fermion
{
    namespace Utils
    {

        static GLenum ShaderTypeFromString(const std::string &type)
        {
            if (type == "vertex")
                return GL_VERTEX_SHADER;
            if (type == "fragment" || type == "pixel")
                return GL_FRAGMENT_SHADER;

            FMAssert::Assert(false, "Unknown shader type", __FILE__, __LINE__);
            return 0;
        }

    }
    OpenGLShader::OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc)
    {
        compile(vertexSrc, fragmentSrc);
    }

    OpenGLShader::OpenGLShader(const std::string &filepath)
    {
        std::string source = readFile(filepath);
        auto shaderSources = preProcess(source);

        const std::string &vertexSrc = shaderSources[GL_VERTEX_SHADER];
        const std::string &fragmentSrc = shaderSources[GL_FRAGMENT_SHADER];

        // 编译着色器
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

    std::string OpenGLShader::readFile(const std::string &filepath)
    {

        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size != -1)
            {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], size);
            }
            else
            {
                Log::Error("Failed to read file :" + filepath);
            }
        }
        else
        {
            Log::Error("Failed to open file : " + filepath);
        }

        return result;
    }
    std::unordered_map<uint32_t, std::string> OpenGLShader::preProcess(const std::string &source)
    {

        std::unordered_map<GLenum, std::string> shaderSources;

        const char *typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);
            FMAssert::Assert(eol != std::string::npos, "Syntax error", __FILE__, __LINE__);
            size_t begin = pos + typeTokenLength + 1; // Start of shader type name (after "#type " keyword)
            std::string type = source.substr(begin, eol - begin);

            FMAssert::Assert(Utils::ShaderTypeFromString(type), "Invalid shader type specified", __FILE__, __LINE__);

            size_t nextLinePos = source.find_first_not_of("\r\n", eol); // Start of shader code after shader type declaration line
            FMAssert::Assert(nextLinePos != std::string::npos, "Syntax error", __FILE__, __LINE__);
            pos = source.find(typeToken, nextLinePos); // Start of next shader type declaration line

            shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }

        return shaderSources;
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
            Log::Warn("Uniform " + name + " not found");
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
    void OpenGLShader::setFloat3(const std::string &name, const glm::vec3 &value)
    {
        glUniform3f(getUniformLocation(name), value.x, value.y, value.z);
    }

    void OpenGLShader::setFloat4(const std::string &name, float v0, float v1, float v2, float v3)
    {
        glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
    }

    void OpenGLShader::setFloat4(const std::string &name, const glm::vec4 &value)
    {
        glUniform4f(getUniformLocation(name), value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::setMat4(const std::string &name, const glm::mat4 &matrix)
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

}