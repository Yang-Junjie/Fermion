#pragma once
#include "fmpch.hpp"
#include "Shader.hpp"
namespace Fermion {
enum class CullMode {
    None,
    Front,
    Back
};

enum class DepthCompareOperator {
    None = 0,
    Never,
    NotEqual,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Equal,
    Always,
};
struct PipelineSpecification {
    std::shared_ptr<Shader> Shader;
    bool DepthTest = true;
    bool DepthWrite = true;

    CullMode Cull = CullMode::Back;
    DepthCompareOperator DepthOperator = DepthCompareOperator::Less;
};
class Pipeline {
public:
    virtual ~Pipeline() = default;

    virtual PipelineSpecification &GetSpecification() = 0;
    virtual const PipelineSpecification &GetSpecification() const = 0;

    virtual void Bind() = 0;

    virtual std::shared_ptr<Shader> GetShader() const = 0;

    static std::shared_ptr<Pipeline> Create(const PipelineSpecification &spec);
};

} // namespace Fermion