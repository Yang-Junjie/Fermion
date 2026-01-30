#pragma once
#include "Asset/Asset.hpp"
#include "Renderer/Model/Material.hpp"
#include <glm/glm.hpp>
#include <imgui.h>

namespace Fermion
{
    class MaterialEditorPanel
    {
    public:
        MaterialEditorPanel() = default;
        ~MaterialEditorPanel() = default;

        void setPanelOpenState(bool state);
        void clearData();

        void onImGuiRender();

    private:
        // TODO: 找一个好一点的位置放（后期对imgui封装）
        bool InputTextStdString(const char *label, std::string &str)
        {
            char buffer[512];
            std::strcpy(buffer, str.c_str());
            if (ImGui::InputText(label, buffer, sizeof(buffer)))
            {
                str = buffer;
                return true;
            }
            return false;
        }

        struct MaterialInfo
        {

            std::string Name;
            MaterialType Type;
            // Phong
            glm::vec4 DiffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            glm::vec4 AmbientColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            AssetHandle DiffuseTextureHandle = AssetHandle(0);

            // PBR
            glm::vec3 Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
            float Metallic = 0.0f;
            float Roughness = 0.5f;
            float AO = 1.0f;

            MapAssets Maps;
        };
        MaterialInfo m_MaterialInfo;
        bool m_isCreatePBRMaterial = true;
        bool m_useTexture = false;
        bool m_isOpenned = false;
    };
} // namespace Fermion
