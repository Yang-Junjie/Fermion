#pragma once
#include <glm/glm.hpp>

namespace Fermion
{
    // UBO binding points - must match shader layout(binding=N) declarations
    namespace UniformBufferBinding
    {
        constexpr uint32_t Camera = 0;      // Camera/view-projection data
        constexpr uint32_t Model = 1;       // Per-object model transforms
        constexpr uint32_t Lights = 2;      // Scene lighting data
        constexpr uint32_t Material = 3;    // Material properties
    }

    // Camera uniform buffer (binding = 0)
    // Contains per-frame camera data
    struct CameraData
    {
        glm::mat4 viewProjection;  // 64 bytes
        glm::mat4 view;            // 64 bytes
        glm::mat4 projection;      // 64 bytes
        glm::vec3 position;        // 12 bytes
        float _padding0;           // 4 bytes (align to 16)

        static constexpr uint32_t getSize() { return 208; }
    };

    // Model uniform buffer (binding = 1)
    // Contains per-object transform data
    struct ModelData
    {
        glm::mat4 model;           // 64 bytes
        glm::mat4 normalMatrix;    // 64 bytes (transpose(inverse(model)))
        int objectID;              // 4 bytes
        float _padding0[3];        // 12 bytes (align to 16)

        static constexpr uint32_t getSize() { return 144; }
    };

    // Light uniform buffer (binding = 2)
    // Contains scene lighting information
    struct LightData
    {
        glm::mat4 lightSpaceMatrix;  // 64 bytes (for shadow mapping)

        // Main directional light (for shadow mapping)
        glm::vec3 dirLightDirection; // 12 bytes
        float dirLightIntensity;     // 4 bytes
        glm::vec3 dirLightColor;     // 12 bytes
        float _padding0;             // 4 bytes

        // Shadow settings
        float shadowBias;            // 4 bytes
        float shadowSoftness;        // 4 bytes
        int enableShadows;           // 4 bytes (bool as int)
        int numDirLights;            // 4 bytes (number of directional lights)

        // Ambient and other settings
        float ambientIntensity;      // 4 bytes
        int numPointLights;          // 4 bytes
        int numSpotLights;           // 4 bytes
        float _padding2;             // 4 bytes

        static constexpr uint32_t getSize() { return 128; }
    };

    // Material uniform buffer (binding = 3)
    // Contains material properties for PBR rendering
    struct MaterialData
    {
        glm::vec4 albedo;            // 16 bytes
        float metallic;              // 4 bytes
        float roughness;             // 4 bytes
        float ao;                    // 4 bytes (ambient occlusion)
        float _padding0;             // 4 bytes

        // Texture flags (0 = no texture, 1 = has texture)
        int hasAlbedoMap;            // 4 bytes
        int hasNormalMap;            // 4 bytes
        int hasMetallicMap;          // 4 bytes
        int hasRoughnessMap;         // 4 bytes

        static constexpr uint32_t getSize() { return 48; }
    };

} // namespace Fermion
