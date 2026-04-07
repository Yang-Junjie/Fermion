#include "Pipeline.hpp"
#include "RendererAPI.hpp"
#include "OpenGLPipeline.hpp"
namespace Fermion {

std::shared_ptr<Pipeline> Pipeline::create(const PipelineSpecification &spec) {
    switch (RendererAPI::getAPI()) {
    case RendererAPI::API::None:
        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLPipeline>(spec);
    case RendererAPI::API::Vulkan:
        FERMION_ASSERT(false, "Vulkan pipeline creation is not implemented yet.");
        return nullptr;
    }
    FERMION_ASSERT(false, "Unknown RendererAPI");
    return nullptr;
}

} // namespace Fermion
