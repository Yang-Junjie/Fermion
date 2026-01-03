#include "OpenGLPipeline.hpp"
#include "glad/glad.h"
namespace Fermion
{
    static GLenum ToGLCompare(DepthCompareOperator op)
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

    static GLenum ToGLCull(CullMode cull)
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
    OpenGLPipeline::OpenGLPipeline(const PipelineSpecification &spec) : m_Specification(spec)
    {
    }
    PipelineSpecification &OpenGLPipeline::GetSpecification()
    {
        return m_Specification;
    }
    const PipelineSpecification &OpenGLPipeline::GetSpecification() const
    {
        return m_Specification;
    }
    void OpenGLPipeline::Bind()
    {
        if (m_Specification.Shader)
            m_Specification.Shader->bind();

        // Depth
        if (m_Specification.DepthTest)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        glDepthMask(m_Specification.DepthWrite ? GL_TRUE : GL_FALSE);

        // Depth compare
        glDepthFunc(ToGLCompare(m_Specification.DepthOperator));

        // Cull
        switch (m_Specification.Cull)
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
    std::shared_ptr<Shader> OpenGLPipeline::GetShader() const
    {
        return m_Specification.Shader;
    }
} // namespace Fermion
