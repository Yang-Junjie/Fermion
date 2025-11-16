#include "Font.hpp"

#include <fstream>
#include <sstream>

namespace Fermion
{
    namespace
    {
        bool startsWith(const std::string &line, const char *prefix)
        {
            return line.rfind(prefix, 0) == 0;
        }

        bool parseKeyValue(const std::string &pair, std::string &key, std::string &value)
        {
            size_t eq = pair.find('=');
            if (eq == std::string::npos)
                return false;
            key = pair.substr(0, eq);
            value = pair.substr(eq + 1);
            return true;
        }
    }

    std::shared_ptr<Font> Font::create(const std::string &atlasPath, const std::string &metricsPath)
    {
        auto font = std::shared_ptr<Font>(new Font());
        if (!font->load(atlasPath, metricsPath))
            return nullptr;
        return font;
    }

    const FontGlyph *Font::getGlyph(uint32_t codepoint) const
    {
        auto it = m_glyphs.find(codepoint);
        if (it == m_glyphs.end())
            return nullptr;
        return &it->second;
    }

    bool Font::load(const std::string &atlasPath, const std::string &metricsPath)
    {
        m_atlasTexture = Texture2D::create(atlasPath);
        if (!m_atlasTexture || !m_atlasTexture->isLoaded())
            return false;

        std::ifstream in(metricsPath);
        if (!in.is_open())
            return false;

        const float texW = static_cast<float>(m_atlasTexture->getWidth());
        const float texH = static_cast<float>(m_atlasTexture->getHeight());

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            if (startsWith(line, "common"))
            {
                // Common line, e.g.:
                // common lineHeight=32 base=26 scaleW=256 scaleH=256 pages=1 packed=0
                std::istringstream ss(line);
                std::string token;
                while (ss >> token)
                {
                    std::string key, value;
                    if (!parseKeyValue(token, key, value))
                        continue;

                    if (key == "lineHeight")
                    {
                        m_lineHeight = static_cast<float>(std::stof(value));
                    }
                }
            }
            else if (startsWith(line, "char "))
            {
                // Char line, BMFont text format, e.g.:
                // char id=65 x=34 y=50 width=18 height=20 xoffset=1 yoffset=6 xadvance=19 ...
                std::istringstream ss(line);
                std::string token;

                uint32_t id = 0;
                float x = 0.0f;
                float y = 0.0f;
                float w = 0.0f;
                float h = 0.0f;
                float xAdvance = 0.0f;

                // skip the initial "char"
                ss >> token;
                while (ss >> token)
                {
                    std::string key, value;
                    if (!parseKeyValue(token, key, value))
                        continue;

                    if (key == "id")
                        id = static_cast<uint32_t>(std::stoul(value));
                    else if (key == "x")
                        x = static_cast<float>(std::stof(value));
                    else if (key == "y")
                        y = static_cast<float>(std::stof(value));
                    else if (key == "width")
                        w = static_cast<float>(std::stof(value));
                    else if (key == "height")
                        h = static_cast<float>(std::stof(value));
                    else if (key == "xadvance")
                        xAdvance = static_cast<float>(std::stof(value));
                }

                if (w <= 0.0f || h <= 0.0f)
                    continue;

                FontGlyph glyph;
                glyph.size = {w, h};
                glyph.advance = xAdvance > 0.0f ? xAdvance : w;

                // BMFont uses top-left origin for x,y
                glm::vec2 uvMin{x / texW, y / texH};
                glm::vec2 uvMax{(x + w) / texW, (y + h) / texH};
                glyph.subTexture = std::make_shared<SubTexture2D>(m_atlasTexture, uvMin, uvMax);

                m_glyphs[id] = std::move(glyph);
            }
        }

        return !m_glyphs.empty();
    }
}

