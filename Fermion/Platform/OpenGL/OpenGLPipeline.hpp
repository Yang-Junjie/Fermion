#pragma once
#include "Renderer/Pipeline.hpp"
namespace Fermion {

class OpenGLPipeline : public Pipeline {
public:
    OpenGLPipeline(const PipelineSpecification &spec);
    virtual PipelineSpecification &GetSpecification() override;
    virtual const PipelineSpecification &GetSpecification() const override;
    virtual void Bind() override;
    virtual std::shared_ptr<Shader> GetShader() const override;
    virtual ~OpenGLPipeline() = default;

private:
    PipelineSpecification m_Specification;
};
} // namespace Fermion
