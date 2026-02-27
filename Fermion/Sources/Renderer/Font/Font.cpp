#include "fmpch.hpp"
#include "Font.hpp"
#include "../Texture/Texture.hpp"
#include "Renderer/Font/MSDFData.hpp"
#include "Project/Project.hpp"

#include <stb_image_write.h>
#undef INFINITE
#include <msdf-atlas-gen.h>
#include <FontGeometry.h>
#include <GlyphGeometry.h>


namespace Fermion
{
    template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static std::shared_ptr<Texture2D> createAndCacheAtlas(const std::filesystem::path &outputPath, const std::string &fontName, float fontSize,
                                                          const std::vector<msdf_atlas::GlyphGeometry> &glyphs,
                                                          const msdf_atlas::FontGeometry &fontGeometry, uint32_t width,
                                                          uint32_t height)
    {
        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(
            width, height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);
        generator.generate(glyphs.data(), (int)glyphs.size());

        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

        if (!std::filesystem::exists(outputPath))
        {
            // 翻转图像数据以匹配 OpenGL 纹理加载时的翻转
            int rowSize = bitmap.width * N;
            std::vector<T> flippedPixels(bitmap.width * bitmap.height * N);

            for (int y = 0; y < bitmap.height; ++y)
            {
                const T *srcRow = bitmap.pixels + y * bitmap.width * N;
                T *dstRow = flippedPixels.data() + (bitmap.height - 1 - y) * bitmap.width * N;
                std::memcpy(dstRow, srcRow, rowSize * sizeof(T));
            }

            stbi_write_png(
                outputPath.string().c_str(),
                bitmap.width,
                bitmap.height,
                N,
                flippedPixels.data(),
                bitmap.width * N);
        }

        TextureSpecification spec;
        spec.Width = bitmap.width;
        spec.Height = bitmap.height;
        spec.Format = ImageFormat::RGB8;
        spec.GenerateMips = false;

        std::shared_ptr<Texture2D> texture = Texture2D::create(spec);
        texture->setData((void *)bitmap.pixels, bitmap.width * bitmap.height * 3);
        return texture;
    }

    Font::Font(const std::filesystem::path &filepath) : m_data(new MSDFData())
    {
        // 初始化 Freetype
        msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype();
        FERMION_ASSERT(ft, "Failed to initialize freetype");

        std::string fileString = filepath.string();

        // 加载字体文件
        msdfgen::FontHandle *font = msdfgen::loadFont(ft, fileString.c_str());
        if (!font)
        {
            Log::Error(std::format("Failed to load font: {}", fileString));
            msdfgen::deinitializeFreetype(ft);
            return;
        }

        // 定义字符集
        struct CharsetRange
        {
            uint32_t Begin, End;
        };

        static const CharsetRange charsetRanges[] = {{0x0020, 0x00FF}};

        msdf_atlas::Charset charset;
        for (CharsetRange range : charsetRanges)
        {
            for (uint32_t c = range.Begin; c <= range.End; c++)
                charset.add(c);
        }

        // 加载字形几何信息（必需，用于文本渲染）
        double fontScale = 1.0;
        m_data->fontGeometry = msdf_atlas::FontGeometry(&m_data->glyphs);
        int glyphsLoaded = m_data->fontGeometry.loadCharset(font, fontScale, charset);
        Log::Info(std::format("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size()));

        double emSize = 40.0;

        // 打包字形到 atlas
        msdf_atlas::TightAtlasPacker atlasPacker;
        atlasPacker.setPixelRange(2.0);
        atlasPacker.setMiterLimit(1.0);
        atlasPacker.setScale(emSize);
        int remaining = atlasPacker.pack(m_data->glyphs.data(), (int)m_data->glyphs.size());
        FERMION_ASSERT(remaining == 0, "Failed to pack font");

        int width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

        // 检查是否存在同名 PNG 文件
        std::filesystem::path pngPath = filepath;
        pngPath.replace_extension(".png");

        if (std::filesystem::exists(pngPath))
        {
            // PNG 已存在，直接加载纹理
            Log::Info(std::format("Loading cached font atlas: {}", pngPath.string()));
            m_atlasTexture = Texture2D::create(pngPath.string(), false);
        }
        else
        {
            // PNG 不存在，生成 MSDF atlas
            Log::Info(std::format("Generating MSDF atlas for font: {}", filepath.string()));

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

            uint64_t coloringSeed = 0;
            bool expensiveColoring = false;
            if (expensiveColoring)
            {
                msdf_atlas::Workload([&glyphs = m_data->glyphs, &coloringSeed](int i, int threadNo) -> bool
                                     {
                            unsigned long long glyphSeed =
                                    (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
                            glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                            return true; }, m_data->glyphs.size())
                    .finish(THREAD_COUNT);
            }
            else
            {
                unsigned long long glyphSeed = coloringSeed;
                for (msdf_atlas::GlyphGeometry &glyph : m_data->glyphs)
                {
                    glyphSeed *= LCG_MULTIPLIER;
                    glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                }
            }

            m_atlasTexture = createAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(
                pngPath, "Test", (float)emSize, m_data->glyphs, m_data->fontGeometry, width, height);
        }

#if 0
        msdfgen::Shape shape;
        if (msdfgen::loadGlyph(shape, font, 'C')) {
            shape.normalize();
            //                      max. angle
            msdfgen::edgeColoringSimple(shape, 3.0);
            //           image width, height
            msdfgen::Bitmap<float, 3> msdf(32, 32);
            //                     range, scale, translation
            msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
            msdfgen::savePng(msdf, "output.png");
        }
#endif

        msdfgen::destroyFont(font);
        msdfgen::deinitializeFreetype(ft);
    }

    Font::~Font()
    {
        delete m_data;
    }

    std::shared_ptr<Font> Font::getDefault()
    {
        static std::shared_ptr<Font> DefaultFont;
        if (!DefaultFont)
        {
            // 尝试从项目配置加载默认字体
            auto project = Project::getActive();
            if (project)
            {
                const auto &defaultFontPath = project->getConfig().defaultFont;
                if (!defaultFontPath.empty() && std::filesystem::exists(defaultFontPath))
                {
                    DefaultFont = std::make_shared<Font>(defaultFontPath);
                    return DefaultFont;
                }
            }

            // 回退到硬编码路径
            Log::Warn("No default font specified in project config, using fallback path");
            DefaultFont = std::make_shared<Font>("../Boson/Resources/assets/fonts/Play-Regular.ttf");
        }

        return DefaultFont;
    }
    std::shared_ptr<Texture2D> Font::getAtlasTexture() const
    {
        return m_atlasTexture;
    }
} // namespace Fermion
