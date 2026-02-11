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

enum class BlendFactor {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

enum class BlendFunction {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

struct PipelineSpecification {
    std::shared_ptr<Shader> shader;
    bool depthTest = true;
    bool depthWrite = true;

    CullMode cull = CullMode::Back;
    DepthCompareOperator depthOperator = DepthCompareOperator::Less;

    bool blendEnable = false;
    BlendFactor srcColorFactor = BlendFactor::SrcAlpha;
    BlendFactor dstColorFactor = BlendFactor::OneMinusSrcAlpha;
    BlendFunction colorBlendFunction = BlendFunction::Add;
    BlendFactor srcAlphaFactor = BlendFactor::One;
    BlendFactor dstAlphaFactor = BlendFactor::OneMinusSrcAlpha;
    BlendFunction alphaBlendFunction = BlendFunction::Add;
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