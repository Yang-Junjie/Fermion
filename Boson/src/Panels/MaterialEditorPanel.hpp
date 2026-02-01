#pragma once
#include "Asset/Asset.hpp"
#include "Renderer/Model/Material.hpp"
#include "Renderer/Preview/MaterialPreviewRenderer.hpp"
#include <glm/glm.hpp>
#include <imgui.h>
#include <vector>
#include <string>
#include <memory>

namespace ax
{
    namespace NodeEditor
    {
        struct EditorContext;
    }
}

namespace Fermion
{
    class MaterialEditorPanel
    {
    public:
        MaterialEditorPanel();
        ~MaterialEditorPanel();

        void setPanelOpenState(bool state);
        void clearData();

        void onImGuiRender();

        // Pin ID conventions:
        // PBR Output node (ID=1) input pins: 1*100+pinIndex (101=Albedo, 102=Normal, 103=Metallic, 104=Roughness, 105=AO)
        // Texture node (ID=N, N>=2) output pin: N*100+1
        static constexpr int PBR_OUTPUT_NODE_ID = 1;
        static int inputPinID(int pinIndex) { return PBR_OUTPUT_NODE_ID * 100 + pinIndex; }
        static int outputPinID(int nodeID) { return nodeID * 100 + 1; }

        enum PBRInputPin : int
        {
            Albedo = 1,
            Normal = 2,
            Metallic = 3,
            Roughness = 4,
            AO = 5
        };

    private:
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

        struct TextureNodeInfo
        {
            int ID = 0;
            AssetHandle TextureHandle = AssetHandle(0);
            float PosX = 0.0f;
            float PosY = 0.0f;
        };

        struct LinkInfo
        {
            int ID = 0;
            int StartPinID = 0;
            int EndPinID = 0;
        };

        // PBR Output node values
        glm::vec3 m_Albedo = glm::vec3(1.0f);
        float m_Metallic = 0.0f;
        float m_Roughness = 0.5f;
        float m_AO = 1.0f;
        float m_OutputNodePosX = 400.0f;
        float m_OutputNodePosY = 100.0f;

        // Graph data
        std::vector<TextureNodeInfo> m_TextureNodes;
        std::vector<LinkInfo> m_Links;
        int m_NextNodeID = 2;
        int m_NextLinkID = 1;

        std::string m_MaterialName = "Material";

        ax::NodeEditor::EditorContext *m_EditorContext = nullptr;
        bool m_isOpenned = false;
        bool m_FirstFrame = true;

        // Preview rendering
        std::unique_ptr<MaterialPreviewRenderer> m_PreviewRenderer;
        std::unique_ptr<Texture2D> m_PreviewTexture;
        bool m_NeedUpdatePreview = true;

        void drawOutputNode();
        void drawTextureNode(TextureNodeInfo &node);
        float drawNodeHeader(const char *title, float minWidth);
        void handleInteractions();
        bool isPinLinked(int pinID) const;
        AssetHandle getLinkedTextureHandle(int inputPinID) const;
        void compileMaterial();
        void loadFromMaterial(std::shared_ptr<Material> material);
        MaterialNodeEditorData buildEditorData() const;
        void loadEditorData(const MaterialNodeEditorData &data);
        void updatePreview();
        std::shared_ptr<Material> buildMaterialFromGraph() const;
    };
} // namespace Fermion
