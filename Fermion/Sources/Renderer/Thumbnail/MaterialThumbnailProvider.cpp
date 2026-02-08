#include "MaterialThumbnailProvider.hpp"
#include "Renderer/Model/MaterialSerializer.hpp"
#include "Core/Log.hpp"

namespace Fermion
{

    MaterialThumbnailProvider::MaterialThumbnailProvider()
    {
        m_renderer = std::make_unique<MaterialPreviewRenderer>();
        if (m_renderer && m_renderer->isInitialized())
        {
            Log::Info("MaterialThumbnailProvider: Initialized successfully");
        }
        else
        {
            Log::Warn("MaterialThumbnailProvider: Renderer not fully initialized yet");
        }
    }

    std::unique_ptr<Texture2D> MaterialThumbnailProvider::generateThumbnail(
        const std::filesystem::path &path)
    {
        if (!m_renderer)
        {
            Log::Warn("MaterialThumbnailProvider: Renderer is null");
            return nullptr;
        }

        if (!std::filesystem::exists(path))
        {
            Log::Warn("MaterialThumbnailProvider: File not found: " + path.string());
            return nullptr;
        }

        // Load the material from file
        auto material = MaterialSerializer::deserialize(path);
        if (!material)
        {
            Log::Warn("MaterialThumbnailProvider: Failed to load material: " + path.string());
            return nullptr;
        }

        // Render the preview
        PreviewSettings settings;
        settings.width = 128;
        settings.height = 128;

        auto result = m_renderer->renderPreview(material, settings);
        if (!result)
        {
            Log::Warn("MaterialThumbnailProvider: Failed to render preview for: " + path.string());
        }

        return result;
    }

} // namespace Fermion
