#pragma once
#include "Renderer/Pipeline.hpp"
namespace Fermion {

class OpenGLPipeline : public Pipeline {
public:
    OpenGLPipeline(const PipelineSpecification &spec);
    virtual PipelineSpecification &getSpecification() override;
    virtual const PipelineSpecification &getSpecification() const override;
    virtual void bind() override;
    virtual std::shared_ptr<Shader> getShader() const override;
    virtual ~OpenGLPipeline() = default;

private:
    PipelineSpecification m_specification;
};
} // namespace Fermion
