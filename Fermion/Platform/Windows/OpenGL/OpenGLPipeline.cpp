#include "OpenGLPipeline.hpp"
#include "glad/glad.h"
namespace Fermion
{
    static GLenum toGLCompare(DepthCompareOperator op)
    {
        switch (op)
        {
        case DepthCompareOperator::Never:
            return GL_NEVER;
        case DepthCompareOperator::NotEqual:
            return GL_NOTEQUAL;
        case DepthCompareOperator::Less:
            return GL_LESS;
        case DepthCompareOperator::LessOrEqual:
            return GL_LEQUAL;
        case DepthCompareOperator::Greater:
            return GL_GREATER;
        case DepthCompareOperator::GreaterOrEqual:
            return GL_GEQUAL;
        case DepthCompareOperator::Equal:
            return GL_EQUAL;
        case DepthCompareOperator::Always:
            return GL_ALWAYS;
        default:
            return GL_LESS;
        }
    }

    static GLenum toGLCull(CullMode cull)
    {
        switch (cull)
        {
        case CullMode::Front:
            return GL_FRONT;
        case CullMode::Back:
            return GL_BACK;
        case CullMode::None:
            return GL_BACK;
        default:
            return GL_BACK;
        }
    }
    OpenGLPipeline::OpenGLPipeline(const PipelineSpecification &spec) : m_specification(spec)
    {
    }
    PipelineSpecification &OpenGLPipeline::getSpecification()
    {
        return m_specification;
    }
    const PipelineSpecification &OpenGLPipeline::getSpecification() const
    {
        return m_specification;
    }
    void OpenGLPipeline::bind()
    {
        if (m_specification.shader)
            m_specification.shader->bind();

        // Depth
        if (m_specification.depthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        glDepthMask(m_specification.depthWrite ? GL_TRUE : GL_FALSE);

        // Depth compare
        glDepthFunc(toGLCompare(m_specification.depthOperator));

        // Cull
        switch (m_specification.cull)
        {
        case CullMode::None:
            glDisable(GL_CULL_FACE);
            break;

        case CullMode::Front:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;

        case CullMode::Back:
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        }
    }
    std::shared_ptr<Shader> OpenGLPipeline::getShader() const
    {
        return m_specification.shader;
    }
} // namespace Fermion
