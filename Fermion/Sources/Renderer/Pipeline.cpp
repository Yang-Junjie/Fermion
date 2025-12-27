#include "Pipeline.hpp"
#include "RendererAPI.hpp"
#include "OpenGLPipeline.hpp"
namespace Fermion {

std::shared_ptr<Pipeline> Pipeline::Create(const PipelineSpecification &spec) {
    switch (RendererAPI::getAPI()) {
    case RendererAPI::API::None:
        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLPipeline>(spec);
    }
    FERMION_ASSERT(false, "Unknown RendererAPI");
    return nullptr;
}

} // namespace Fermion