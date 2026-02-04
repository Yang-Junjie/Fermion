#define IMGUI_DEFINE_MATH_OPERATORS
#include "MaterialEditorPanel.hpp"
#include <imgui_node_editor.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer/Model/MaterialFactory.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetManager/EditorAssetManager.hpp"
#include <algorithm>
#include "widgets.h"

namespace ed = ax::NodeEditor;
namespace widgets = ax::Widgets;

namespace Fermion
{
    static ImColor GetPinColor(MaterialEditorPanel::PBRInputPin pin)
    {
        switch (pin)
        {
        case MaterialEditorPanel::Albedo:
            return ImColor(220, 48, 48);
        case MaterialEditorPanel::Normal:
            return ImColor(68, 201, 156);
        case MaterialEditorPanel::Metallic:
            return ImColor(147, 226, 74);
        case MaterialEditorPanel::Roughness:
            return ImColor(124, 21, 153);
        case MaterialEditorPanel::AO:
            return ImColor(51, 150, 215);
        default:
            return ImColor(255, 255, 255);
        }
    }

    static void DrawPinIcon(bool connected, int alpha, MaterialEditorPanel::PBRInputPin pinType)
    {
        using IconType = ax::Drawing::IconType;
        ImColor color = GetPinColor(pinType);
        color.Value.w = alpha / 255.0f;
        widgets::Icon(ImVec2(24, 24), IconType::Circle, connected, color, ImColor(32, 32, 32, alpha));
    }

    static void DrawOutputPinIcon(bool connected, int alpha)
    {
        using IconType = ax::Drawing::IconType;
        widgets::Icon(ImVec2(24, 24), IconType::Circle, connected, ImColor(255, 255, 255, alpha), ImColor(32, 32, 32, alpha));
    }

    MaterialEditorPanel::MaterialEditorPanel()
    {
        ed::Config config;
        config.SettingsFile = "";
        m_EditorContext = ed::CreateEditor(&config);
        m_PreviewRenderer = std::make_unique<MaterialPreviewRenderer>();
    }

    MaterialEditorPanel::~MaterialEditorPanel()
    {
        if (m_EditorContext)
        {
            ed::DestroyEditor(m_EditorContext);
            m_EditorContext = nullptr;
        }
    }

    void MaterialEditorPanel::setPanelOpenState(bool state)
    {
        m_isOpenned = state;
        if (state)
            m_FirstFrame = true;
    }

    void MaterialEditorPanel::clearData()
    {
        m_Albedo = glm::vec3(1.0f);
        m_Metallic = 0.0f;
        m_Roughness = 0.5f;
        m_AO = 1.0f;
        m_OutputNodePosX = 400.0f;
        m_OutputNodePosY = 100.0f;
        m_TextureNodes.clear();
        m_Links.clear();
        m_NextNodeID = 2;
        m_NextLinkID = 1;
        m_MaterialName = "Material";
        m_FirstFrame = true;
        m_NeedUpdatePreview = true;
        m_PreviewTexture.reset();
    }

    void MaterialEditorPanel::onImGuiRender()
    {
        if (!m_isOpenned)
            return;

        ed::SetCurrentEditor(m_EditorContext);

        if (!ImGui::Begin("Material Editor", &m_isOpenned))
        {
            ImGui::End();
            ed::SetCurrentEditor(nullptr);
            return;
        }

        // Top bar
        ImGui::SetNextItemWidth(150.0f);
        InputTextStdString("##Name", m_MaterialName);
        ImGui::SameLine();

        if (ImGui::Button("New"))
            clearData();
        ImGui::SameLine();

        ImGui::SameLine();
        if (ImGui::Button("Save Material"))
            compileMaterial();

        ImGui::SameLine();
        if (ImGui::Button("Add Texture"))
        {
            int newId = m_NextNodeID++;
            float yOffset = static_cast<float>(m_TextureNodes.size()) * 170.0f;
            m_TextureNodes.push_back({newId, AssetHandle(0), 50.0f, yOffset});
            m_FirstFrame = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Update Preview"))
            m_NeedUpdatePreview = true;

        if (m_NeedUpdatePreview)
        {
            updatePreview();
            m_NeedUpdatePreview = false;
        }

        ImGui::Separator();

        // Node editor
        ed::Begin("MaterialNodeEditor");

        if (m_FirstFrame)
        {
            ed::SetNodePosition(ed::NodeId(PBR_OUTPUT_NODE_ID), ImVec2(m_OutputNodePosX, m_OutputNodePosY));
            for (auto &node : m_TextureNodes)
                ed::SetNodePosition(ed::NodeId(node.ID), ImVec2(node.PosX, node.PosY));
            ed::NavigateToContent();
            m_FirstFrame = false;
        }

        // Draw nodes
        drawOutputNode();
        for (auto &node : m_TextureNodes)
            drawTextureNode(node);

        // Draw links with colors
        for (auto &link : m_Links)
        {
            int endPinIndex = link.EndPinID % 100;
            ImColor linkColor = ImColor(200, 200, 200);
            if (endPinIndex >= Albedo && endPinIndex <= AO)
                linkColor = GetPinColor(static_cast<PBRInputPin>(endPinIndex));
            ed::Link(ed::LinkId(link.ID),
                     ed::PinId(link.StartPinID),
                     ed::PinId(link.EndPinID),
                     linkColor, 2.0f);
        }

        handleInteractions();

        ed::End();
        ImGui::End();
        ed::SetCurrentEditor(nullptr);

        if (!m_isOpenned)
            clearData();
    }

    float MaterialEditorPanel::drawNodeHeader(const char *title, float minWidth)
    {
        ImGui::TextUnformatted(title);
        ImGui::Dummy(ImVec2(minWidth, 4.0f));
        float headerBottomY = ImGui::GetCursorScreenPos().y;
        ImGui::Dummy(ImVec2(0, 6.0f)); // margin below header
        return headerBottomY;
    }

    void MaterialEditorPanel::drawOutputNode()
    {
        const float nodeWidth = 240.0f;

        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        ed::BeginNode(ed::NodeId(PBR_OUTPUT_NODE_ID));

        ImVec2 headerMin = ImGui::GetCursorScreenPos();

        ImGui::BeginGroup();
        float headerMaxY = drawNodeHeader("PBR Material Output", nodeWidth);

        // Material preview
        if (m_PreviewTexture && m_PreviewTexture->isLoaded())
        {
            float previewSize = 128.0f;
            float offsetX = (nodeWidth - previewSize) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
            ImTextureID previewID = (ImTextureID)(uintptr_t)m_PreviewTexture->getRendererID();
            ImGui::Image(previewID, ImVec2(previewSize, previewSize), ImVec2(0, 1), ImVec2(1, 0));
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image(previewID, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::EndTooltip();
            }
        }
        else
        {
            ImGui::Dummy(ImVec2(nodeWidth, 128));
        }

        ImGui::Spacing();

        // Input pins
        auto drawInputPin = [&](PBRInputPin pinType, const char *name, auto valueWidget)
        {
            ed::BeginPin(ed::PinId(inputPinID(pinType)), ed::PinKind::Input);
            DrawPinIcon(isPinLinked(inputPinID(pinType)), 255, pinType);
            ed::EndPin();
            ImGui::SameLine();
            ImGui::TextUnformatted(name);
            if (!isPinLinked(inputPinID(pinType)))
            {
                ImGui::SameLine();
                valueWidget();
            }
        };

        drawInputPin(Albedo, "Albedo", [&]()
                     {
            ImGui::SetNextItemWidth(80);
            ImVec2 btnPos = ImGui::GetCursorScreenPos();
            if (ImGui::ColorButton("##albedo", ImVec4(m_Albedo.r, m_Albedo.g, m_Albedo.b, 1.0f),
                                   ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoBorder, ImVec2(20, 20)))
            {
                m_OpenAlbedoPicker = true;
                m_AlbedoPickerPos = btnPos;
            } });

        drawInputPin(Normal, "Normal", [&]() {
            ImGui::SetNextItemWidth(80);
            ImGui::NewLine();
        });

        drawInputPin(Metallic, "Metallic", [&]()
                     {
            ImGui::SetNextItemWidth(80);
            if (ImGui::SliderFloat("##metallic", &m_Metallic, 0.0f, 1.0f, "%.2f"))
                m_NeedUpdatePreview = true; });

        drawInputPin(Roughness, "Roughness", [&]()
                     {
            ImGui::SetNextItemWidth(80);
            if (ImGui::SliderFloat("##roughness", &m_Roughness, 0.0f, 1.0f, "%.2f"))
                m_NeedUpdatePreview = true; });

        drawInputPin(AO, "AO", [&]()
                     {
            ImGui::SetNextItemWidth(80);
            if (ImGui::SliderFloat("##ao", &m_AO, 0.0f, 1.0f, "%.2f"))
                m_NeedUpdatePreview = true; });

        ImGui::EndGroup();

        ImVec2 groupMin = ImGui::GetItemRectMin();
        ImVec2 groupMax = ImGui::GetItemRectMax();

        ed::EndNode();

        // Draw header background using actual node width
        float pad = 7.0f;
        auto *drawList = ed::GetNodeBackgroundDrawList(ed::NodeId(PBR_OUTPUT_NODE_ID));
        drawList->AddRectFilled(
            ImVec2(groupMin.x - pad, headerMin.y - 3.0f),
            ImVec2(groupMax.x + pad, headerMaxY),
            IM_COL32(128, 16, 16, 230), 12.0f, ImDrawFlags_RoundCornersTop);

        ed::PopStyleVar();
    }

    void MaterialEditorPanel::drawTextureNode(TextureNodeInfo &node)
    {
        const float nodeWidth = 120.0f;

        ImGui::PushID(node.ID);
        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
        ed::BeginNode(ed::NodeId(node.ID));

        ImVec2 headerMin = ImGui::GetCursorScreenPos();

        ImGui::BeginGroup();
        float headerMaxY = drawNodeHeader("Texture2D", nodeWidth);

        // Texture preview
        bool hasTexture = static_cast<uint64_t>(node.TextureHandle) != 0;
        float previewSize = 80.0f;
        float offsetX = (nodeWidth - previewSize) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

        if (hasTexture)
        {
            auto texture = Project::getEditorAssetManager()->getAsset<Texture2D>(node.TextureHandle);
            if (texture && texture->isLoaded())
            {
                ImTextureID textureID = (ImTextureID)(uintptr_t)texture->getRendererID();
                ImGui::Image(textureID, ImVec2(previewSize, previewSize), ImVec2(0, 1), ImVec2(1, 0));
            }
            else
            {
                ImGui::Dummy(ImVec2(previewSize, previewSize));
            }
        }
        else
        {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(pos, pos + ImVec2(previewSize, previewSize), IM_COL32(50, 50, 50, 255), 4.0f);
            drawList->AddRect(pos, pos + ImVec2(previewSize, previewSize), IM_COL32(90, 90, 90, 255), 4.0f);
            ImVec2 center(pos.x + previewSize * 0.5f, pos.y + previewSize * 0.5f);
            drawList->AddLine(ImVec2(center.x - 12, center.y), ImVec2(center.x + 12, center.y), IM_COL32(130, 130, 130, 255), 2.0f);
            drawList->AddLine(ImVec2(center.x, center.y - 12), ImVec2(center.x, center.y + 12), IM_COL32(130, 130, 130, 255), 2.0f);
            ImGui::Dummy(ImVec2(previewSize, previewSize));
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_TEXTURE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0])
                {
                    auto editorAssets = Project::getEditorAssetManager();
                    AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                    if (static_cast<uint64_t>(handle) != 0)
                    {
                        node.TextureHandle = handle;
                        m_NeedUpdatePreview = true;
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Spacing();

        // Output pin - right aligned
        float pinOffset = nodeWidth - 50.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pinOffset);
        ImGui::TextUnformatted("Out");
        ImGui::SameLine();
        ed::BeginPin(ed::PinId(outputPinID(node.ID)), ed::PinKind::Output);
        DrawOutputPinIcon(isPinLinked(outputPinID(node.ID)), 255);
        ed::EndPin();

        ImGui::EndGroup();

        ImVec2 groupMin = ImGui::GetItemRectMin();
        ImVec2 groupMax = ImGui::GetItemRectMax();

        ed::EndNode();

        // Draw header background using actual node width
        float pad = 8.0f;
        auto *bgDrawList = ed::GetNodeBackgroundDrawList(ed::NodeId(node.ID));
        bgDrawList->AddRectFilled(
            ImVec2(groupMin.x - pad, headerMin.y - 4.0f),
            ImVec2(groupMax.x + pad, headerMaxY),
            IM_COL32(0, 80, 120, 230), 12.0f, ImDrawFlags_RoundCornersTop);

        ed::PopStyleVar();
        ImGui::PopID();
    }

    void MaterialEditorPanel::handleInteractions()
    {
        // Handle link creation
        if (ed::BeginCreate())
        {
            ed::PinId startId, endId;
            if (ed::QueryNewLink(&startId, &endId))
            {
                if (startId && endId)
                {
                    int start = static_cast<int>(startId.Get());
                    int end = static_cast<int>(endId.Get());

                    bool startIsOutput = (start / 100 >= 2) && (start % 100 == 1);
                    bool endIsInput = (end / 100 == PBR_OUTPUT_NODE_ID) && (end % 100 >= 1 && end % 100 <= 5);

                    if (!startIsOutput || !endIsInput)
                    {
                        std::swap(start, end);
                        startIsOutput = (start / 100 >= 2) && (start % 100 == 1);
                        endIsInput = (end / 100 == PBR_OUTPUT_NODE_ID) && (end % 100 >= 1 && end % 100 <= 5);
                    }

                    if (startIsOutput && endIsInput)
                    {
                        int pinIndex = end % 100;
                        ImColor linkColor = GetPinColor(static_cast<PBRInputPin>(pinIndex));

                        if (!isPinLinked(end))
                        {
                            if (ed::AcceptNewItem(linkColor, 2.0f))
                            {
                                m_Links.push_back({m_NextLinkID++, start, end});
                                m_NeedUpdatePreview = true;
                            }
                        }
                        else
                        {
                            ed::RejectNewItem(ImVec4(1, 0, 0, 1), 2.0f);
                        }
                    }
                    else
                    {
                        ed::RejectNewItem(ImVec4(1, 0, 0, 1), 2.0f);
                    }
                }
            }
            ed::EndCreate();
        }

        // Handle deletion
        if (ed::BeginDelete())
        {
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                if (ed::AcceptDeletedItem())
                {
                    int linkId = static_cast<int>(deletedLinkId.Get());
                    m_Links.erase(
                        std::remove_if(m_Links.begin(), m_Links.end(),
                                       [linkId](const LinkInfo &l)
                                       { return l.ID == linkId; }),
                        m_Links.end());
                    m_NeedUpdatePreview = true;
                }
            }

            ed::NodeId deletedNodeId;
            while (ed::QueryDeletedNode(&deletedNodeId))
            {
                int nodeId = static_cast<int>(deletedNodeId.Get());
                if (nodeId == PBR_OUTPUT_NODE_ID)
                {
                    ed::RejectDeletedItem();
                }
                else if (ed::AcceptDeletedItem())
                {
                    int outPin = outputPinID(nodeId);
                    m_Links.erase(
                        std::remove_if(m_Links.begin(), m_Links.end(),
                                       [outPin](const LinkInfo &l)
                                       { return l.StartPinID == outPin; }),
                        m_Links.end());
                    m_TextureNodes.erase(
                        std::remove_if(m_TextureNodes.begin(), m_TextureNodes.end(),
                                       [nodeId](const TextureNodeInfo &n)
                                       { return n.ID == nodeId; }),
                        m_TextureNodes.end());
                }
            }
            ed::EndDelete();
        }

        // Context menu and drag-drop
        ed::Suspend();

        // Handle color picker popup
        if (m_OpenAlbedoPicker)
        {
            ImGui::OpenPopup("AlbedoColorPicker");
            m_OpenAlbedoPicker = false;
        }
        if (ImGui::BeginPopup("AlbedoColorPicker"))
        {
            if (ImGui::ColorPicker3("##albedopicker", glm::value_ptr(m_Albedo)))
                m_NeedUpdatePreview = true;
            ImGui::EndPopup();
        }

        if (ed::ShowBackgroundContextMenu())
            ImGui::OpenPopup("NodeEditorContextMenu");
        if (ImGui::BeginPopup("NodeEditorContextMenu"))
        {
            if (ImGui::MenuItem("Add Texture2D Node"))
            {
                int newId = m_NextNodeID++;
                float yOffset = static_cast<float>(m_TextureNodes.size()) * 170.0f;
                m_TextureNodes.push_back({newId, AssetHandle(0), 50.0f, yOffset});
                ed::SetNodePosition(ed::NodeId(newId), ImVec2(50.0f, yOffset));
            }
            ImGui::EndPopup();
        }

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (ImGui::BeginDragDropTargetCustom(window->ContentRegionRect, window->ID))
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_MATERIAL"))
            {
                auto path = std::string(static_cast<const char *>(payload->Data));
                auto editorAssets = Project::getEditorAssetManager();
                AssetHandle handle = editorAssets->importAsset(std::filesystem::path(path));
                if (static_cast<uint64_t>(handle) != 0)
                {
                    auto material = editorAssets->getAsset<Material>(handle);
                    if (material)
                        loadFromMaterial(material);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ed::Resume();
    }

    bool MaterialEditorPanel::isPinLinked(int pinID) const
    {
        for (const auto &link : m_Links)
        {
            if (link.StartPinID == pinID || link.EndPinID == pinID)
                return true;
        }
        return false;
    }

    AssetHandle MaterialEditorPanel::getLinkedTextureHandle(int inPinID) const
    {
        for (const auto &link : m_Links)
        {
            if (link.EndPinID == inPinID)
            {
                int nodeId = link.StartPinID / 100;
                for (const auto &node : m_TextureNodes)
                {
                    if (node.ID == nodeId)
                        return node.TextureHandle;
                }
            }
        }
        return AssetHandle(0);
    }

    void MaterialEditorPanel::compileMaterial()
    {
        MapAssets maps;
        maps.AlbedoMapHandle = getLinkedTextureHandle(inputPinID(Albedo));
        maps.NormalMapHandle = getLinkedTextureHandle(inputPinID(Normal));
        maps.MetallicMapHandle = getLinkedTextureHandle(inputPinID(Metallic));
        maps.RoughnessMapHandle = getLinkedTextureHandle(inputPinID(Roughness));
        maps.AOMapHandle = getLinkedTextureHandle(inputPinID(AO));

        MaterialNodeEditorData editorData = buildEditorData();

        MaterialFactory::createMaterial(
            m_MaterialName, maps, m_Albedo, m_Metallic, m_Roughness, m_AO, editorData);
    }

    void MaterialEditorPanel::loadFromMaterial(std::shared_ptr<Material> material)
    {
        m_MaterialName = material->getName();
        m_Albedo = material->getAlbedo();
        m_Metallic = material->getMetallic();
        m_Roughness = material->getRoughness();
        m_AO = material->getAO();

        const auto &editorData = material->getEditorData();
        if (!editorData.Nodes.empty())
        {
            loadEditorData(editorData);
        }
        else
        {
            m_TextureNodes.clear();
            m_Links.clear();
            m_NextNodeID = 2;
            m_NextLinkID = 1;
            m_OutputNodePosX = 400.0f;
            m_OutputNodePosY = 100.0f;

            auto addTextureNode = [&](AssetHandle handle, int inputPinIndex, float yPos)
            {
                if (static_cast<uint64_t>(handle) != 0)
                {
                    int id = m_NextNodeID++;
                    m_TextureNodes.push_back({id, handle, 50.0f, yPos});
                    m_Links.push_back({m_NextLinkID++, outputPinID(id), inputPinID(inputPinIndex)});
                }
            };

            addTextureNode(material->getAlbedoMap(), Albedo, 0.0f);
            addTextureNode(material->getNormalMap(), Normal, 150.0f);
            addTextureNode(material->getMetallicMap(), Metallic, 300.0f);
            addTextureNode(material->getRoughnessMap(), Roughness, 450.0f);
            addTextureNode(material->getAOMap(), AO, 600.0f);
        }

        m_FirstFrame = true;
        m_NeedUpdatePreview = true;
    }

    MaterialNodeEditorData MaterialEditorPanel::buildEditorData() const
    {
        MaterialNodeEditorData data;
        data.NextNodeID = m_NextNodeID;
        data.NextLinkID = m_NextLinkID;

        {
            MaterialNodeEditorData::NodeData nd;
            nd.ID = PBR_OUTPUT_NODE_ID;
            nd.Type = 0;
            auto pos = ed::GetNodePosition(ed::NodeId(PBR_OUTPUT_NODE_ID));
            nd.PosX = pos.x;
            nd.PosY = pos.y;
            data.Nodes.push_back(nd);
        }

        for (const auto &texNode : m_TextureNodes)
        {
            MaterialNodeEditorData::NodeData nd;
            nd.ID = texNode.ID;
            nd.Type = 1;
            auto pos = ed::GetNodePosition(ed::NodeId(texNode.ID));
            nd.PosX = pos.x;
            nd.PosY = pos.y;
            nd.TextureHandle = texNode.TextureHandle;
            data.Nodes.push_back(nd);
        }

        for (const auto &link : m_Links)
        {
            MaterialNodeEditorData::LinkData ld;
            ld.ID = link.ID;
            ld.StartPinID = link.StartPinID;
            ld.EndPinID = link.EndPinID;
            data.Links.push_back(ld);
        }

        return data;
    }

    void MaterialEditorPanel::loadEditorData(const MaterialNodeEditorData &data)
    {
        m_TextureNodes.clear();
        m_Links.clear();
        m_NextNodeID = data.NextNodeID;
        m_NextLinkID = data.NextLinkID;

        for (const auto &nd : data.Nodes)
        {
            if (nd.Type == 0)
            {
                m_OutputNodePosX = nd.PosX;
                m_OutputNodePosY = nd.PosY;
            }
            else if (nd.Type == 1)
            {
                m_TextureNodes.push_back({nd.ID, nd.TextureHandle, nd.PosX, nd.PosY});
            }
        }

        for (const auto &ld : data.Links)
        {
            m_Links.push_back({ld.ID, ld.StartPinID, ld.EndPinID});
        }

        m_FirstFrame = true;
        m_NeedUpdatePreview = true;
    }

    void MaterialEditorPanel::updatePreview()
    {
        if (!m_PreviewRenderer || !m_PreviewRenderer->isInitialized())
            return;

        auto material = buildMaterialFromGraph();
        if (!material)
            return;

        MaterialPreviewRenderer::PreviewSettings settings;
        settings.width = 256;
        settings.height = 256;

        m_PreviewTexture = m_PreviewRenderer->renderPreview(material, settings);
    }

    std::shared_ptr<Material> MaterialEditorPanel::buildMaterialFromGraph() const
    {
        auto material = std::make_shared<Material>();
        material->setMaterialType(MaterialType::PBR);
        material->setName(m_MaterialName);
        material->setAlbedo(m_Albedo);
        material->setMetallic(m_Metallic);
        material->setRoughness(m_Roughness);
        material->setAO(m_AO);

        material->setAlbedoMap(getLinkedTextureHandle(inputPinID(Albedo)));
        material->setNormalMap(getLinkedTextureHandle(inputPinID(Normal)));
        material->setMetallicMap(getLinkedTextureHandle(inputPinID(Metallic)));
        material->setRoughnessMap(getLinkedTextureHandle(inputPinID(Roughness)));
        material->setAOMap(getLinkedTextureHandle(inputPinID(AO)));

        return material;
    }
} // namespace Fermion
