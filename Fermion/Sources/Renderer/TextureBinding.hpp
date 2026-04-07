#pragma once

#include <cstdint>

namespace Fermion {

// Texture binding points are part of the shader/pipeline layout contract.
// Keep these values in sync with GLSL layout(binding = N) declarations.
namespace TextureBinding {

namespace Material {
constexpr uint32_t BaseColor = 0;
constexpr uint32_t Normal = 1;
constexpr uint32_t Metallic = 2;
constexpr uint32_t Roughness = 3;
constexpr uint32_t AmbientOcclusion = 4;
} // namespace Material

namespace Deferred {
constexpr uint32_t Albedo = 0;
constexpr uint32_t Normal = 1;
constexpr uint32_t Material = 2;
constexpr uint32_t Emissive = 3;
constexpr uint32_t Depth = 4;
} // namespace Deferred

namespace GBufferDebug {
constexpr uint32_t Albedo = 0;
constexpr uint32_t Normal = 1;
constexpr uint32_t Material = 2;
constexpr uint32_t Emissive = 3;
constexpr uint32_t ObjectID = 4;
constexpr uint32_t Depth = 5;
} // namespace GBufferDebug

namespace PostProcess {
constexpr uint32_t Depth = 0;
} // namespace PostProcess

namespace Shadow {
constexpr uint32_t Map = 10;
} // namespace Shadow

namespace IBL {
constexpr uint32_t Irradiance = 11;
constexpr uint32_t Prefilter = 12;
constexpr uint32_t BrdfLut = 13;
} // namespace IBL

namespace Environment {
constexpr uint32_t Source = 0;
} // namespace Environment

namespace Skybox {
constexpr uint32_t Cubemap = 0;
} // namespace Skybox

namespace Renderer2D {
constexpr uint32_t TextureArrayBase = 0;
constexpr uint32_t TextAtlas = 0;
} // namespace Renderer2D

} // namespace TextureBinding

} // namespace Fermion
