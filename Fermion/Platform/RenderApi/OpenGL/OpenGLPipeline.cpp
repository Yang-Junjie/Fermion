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

    static GLenum toGLBlendFactor(BlendFactor factor)
    {
        switch (factor)
        {
        case BlendFactor::Zero:                  return GL_ZERO;
        case BlendFactor::One:                   return GL_ONE;
        case BlendFactor::SrcColor:              return GL_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor:      return GL_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:              return GL_DST_COLOR;
        case BlendFactor::OneMinusDstColor:      return GL_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:              return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:              return GL_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha:      return GL_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:         return GL_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:         return GL_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
        default:                                 return GL_ONE;
        }
    }

    static GLenum toGLBlendFunction(BlendFunction func)
    {
        switch (func)
        {
        case BlendFunction::Add:             return GL_FUNC_ADD;
        case BlendFunction::Subtract:        return GL_FUNC_SUBTRACT;
        case BlendFunction::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        case BlendFunction::Min:             return GL_MIN;
        case BlendFunction::Max:             return GL_MAX;
        default:                             return GL_FUNC_ADD;
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

        // Blend
        if (m_specification.blendEnable)
        {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(
                toGLBlendFactor(m_specification.srcColorFactor),
                toGLBlendFactor(m_specification.dstColorFactor),
                toGLBlendFactor(m_specification.srcAlphaFactor),
                toGLBlendFactor(m_specification.dstAlphaFactor));
            glBlendEquationSeparate(
                toGLBlendFunction(m_specification.colorBlendFunction),
                toGLBlendFunction(m_specification.alphaBlendFunction));
        }
        else
        {
            glDisable(GL_BLEND);
        }
    }
    std::shared_ptr<Shader> OpenGLPipeline::getShader() const
    {
        return m_specification.shader;
    }
} // namespace Fermion
