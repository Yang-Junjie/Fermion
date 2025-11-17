#pragma once

#include <filesystem>

#include "Renderer/Texture.hpp"

namespace Fermion {

	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& font);
		~Font();

		const MSDFData* getMSDFData() const { return m_data; }
		std::shared_ptr<Texture2D> getAtlasTexture() const { return m_atlasTexture; }

		static std::shared_ptr<Font> getDefault();
	private:
		MSDFData* m_data;
		std::shared_ptr<Texture2D> m_atlasTexture;
	};

}
