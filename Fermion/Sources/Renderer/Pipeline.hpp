#pragma once
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
    std::shared_ptr<Shader> shader;
    bool depthTest = true;
    bool depthWrite = true;

    CullMode cull = CullMode::Back;
    DepthCompareOperator depthOperator = DepthCompareOperator::Less;
};
class Pipeline {
public:
    virtual ~Pipeline() = default;

    virtual PipelineSpecification &getSpecification() = 0;
    virtual const PipelineSpecification &getSpecification() const = 0;

    virtual void bind() = 0;

    virtual std::shared_ptr<Shader> getShader() const = 0;

    static std::shared_ptr<Pipeline> create(const PipelineSpecification &spec);
};

} // namespace Fermion