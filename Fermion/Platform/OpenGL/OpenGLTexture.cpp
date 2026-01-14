#include "fmpch.hpp"
#include "OpenGLTexture.hpp"
#include "../../Sources/Renderer/Framebuffer.hpp"

#include <stb_image.h>

namespace Fermion {
namespace Utils {

static GLenum fermionImageFormatToGLDataFormat(ImageFormat format) {
    switch (format) {
    case ImageFormat::RGB8:
        return GL_RGB;
    case ImageFormat::RGBA8:
        return GL_RGBA;
    case ImageFormat::RGBA32F:
        return GL_RGBA;
    case ImageFormat::RGB16F:
        return GL_RGB;
    case ImageFormat::RG16F:
        return GL_RG;
    }

    FERMION_ASSERT(false, "Unknown ImageFormat!");
    return 0;
}

static GLenum fermionImageFormatToGLInternalFormat(ImageFormat format) {
    switch (format) {
    case ImageFormat::RGB8:
        return GL_RGB8;
    case ImageFormat::RGBA8:
        return GL_RGBA8;
    case ImageFormat::RGBA32F:
        return GL_RGBA32F;
    case ImageFormat::RGB16F:
        return GL_RGB16F;
    case ImageFormat::RG16F:
        return GL_RG16F;
    }

    FERMION_ASSERT(false, "Unknown ImageFormat!");
    return 0;
}

} // namespace Utils

OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &specification, bool generateMipmap) : m_specification(specification), m_width(m_specification.Width), m_height(m_specification.Height), m_generateMipmap(generateMipmap) {
    m_internalFormat = Utils::fermionImageFormatToGLInternalFormat(m_specification.Format);
    m_dataFormat = Utils::fermionImageFormatToGLDataFormat(m_specification.Format);

    int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (generateMipmap)
        glGenerateTextureMipmap(m_rendererID);
}

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, bool generateMipmap) : m_width(width), m_height(height), m_generateMipmap(generateMipmap) {
    m_internalFormat = GL_RGBA8;
    m_dataFormat = GL_RGBA;

    int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (generateMipmap)
        glGenerateTextureMipmap(m_rendererID);
}

OpenGLTexture2D::OpenGLTexture2D(const std::string &path, bool generateMipmap) : m_path(path), m_generateMipmap(generateMipmap) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1);

    bool isHDR = stbi_is_hdr(path.c_str());
    void *data = nullptr;
    GLenum dataType = GL_UNSIGNED_BYTE;

    if (isHDR) {
        float *hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        if (!hdrData) {
            Log::Error(std::format("Failed to load HDR texture image: {}, reason: {}", path, std::string(stbi_failure_reason())));
            return;
        }
        data = hdrData;
        dataType = GL_FLOAT;

        if (channels == 3) {
            m_internalFormat = GL_RGB16F;
            m_dataFormat = GL_RGB;
        } else if (channels == 4) {
            m_internalFormat = GL_RGBA32F;
            m_dataFormat = GL_RGBA;
        } else {
            Log::Error(std::format("Unsupported HDR channel count: {}", channels));
            stbi_image_free(hdrData);
            return;
        }
    } else {
        stbi_uc *ldrData = stbi_load(path.c_str(), &width, &height, &channels, 4);
        if (!ldrData) {
            Log::Error(std::format("Failed to load texture image: {}, reason: {}", path, std::string(stbi_failure_reason())));
            return;
        }
        data = ldrData;
        dataType = GL_UNSIGNED_BYTE;
        m_internalFormat = GL_RGBA8;
        m_dataFormat = GL_RGBA;
    }

    m_isLoaded = true;
    m_width = width;
    m_height = height;

    int levels = generateMipmap ? 1 + (int)std::floor(std::log2(std::max(m_width, m_height))) : 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, levels, m_internalFormat, m_width, m_height);

    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTextureSubImage2D(
        m_rendererID, 0,
        0, 0,
        m_width, m_height,
        m_dataFormat,
        dataType,
        data);

    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, generateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (generateMipmap)
        glGenerateTextureMipmap(m_rendererID);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    FM_PROFILE_FUNCTION();

    glDeleteTextures(1, &m_rendererID);
}

void OpenGLTexture2D::setData(void *data, uint32_t size) {
    FM_PROFILE_FUNCTION();

    uint32_t bpp = m_dataFormat == GL_RGBA ? 4 : 3;
    FERMION_ASSERT(size == m_width * m_height * bpp, "Data must be entire texture!");

    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTextureSubImage2D(
        m_rendererID, 0,
        0, 0,
        m_width, m_height,
        m_dataFormat,
        GL_UNSIGNED_BYTE,
        data);

    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    FM_PROFILE_FUNCTION();

    glBindTextureUnit(slot, m_rendererID);
}

OpenGLTextureCube::OpenGLTextureCube(const std::string &path) : m_path(path) {
    bool isHDR = stbi_is_hdr(path.c_str());
    
    if (isHDR) {
        Log::Info(std::format("Detected HDR file, use equirectangular to cubemap conversion: {}", path));
       
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
        m_width = 512;  
        m_height = 512;
        
        glTextureStorage2D(m_rendererID, 1, GL_RGB16F, m_width, m_height);
        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        Log::Warn("HDR cubemap conversion requires shader processing - please use TextureCube::create with TextureCubeSpecification for runtime conversion");
        m_isLoaded = false;
        return;
    }
    
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);

    std::vector<std::string> faces = {
        "right.jpg", "left.jpg",
        "up.jpg", "down.jpg",
        "front.jpg", "back.jpg"};

    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    bool storageAllocated = false;
    
    for (uint32_t i = 0; i < faces.size(); i++) {
        std::string facePath = path + "/" + faces[i];
        stbi_uc *data = stbi_load(facePath.c_str(), &width, &height, &channels, 4);
        if (!data) {
            Log::Error(std::format("Failed to load cubemap texture at path: {}", facePath));
            continue;
        }

        // 只在第一次分配存储
        if (!storageAllocated) {
            glTextureStorage2D(m_rendererID, 1, GL_RGBA8, width, height);
            storageAllocated = true;
        }
        
        glTextureSubImage3D(
            m_rendererID,
            0,
            0, 0, i, // xoffset, yoffset, zoffset = face index
            width, height, 1,
            GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_width = width;
    m_height = height;
    m_isLoaded = true;
}

OpenGLTextureCube::OpenGLTextureCube(const TextureCubeSpecification &spec) : m_cubeSpec(spec)
{
    if (!spec.names.empty()) {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
        stbi_set_flip_vertically_on_load(spec.flip);
        int width, height, channels;
        
        bool storageAllocated = false;
        for (auto &[face, name] : spec.names) {
            std::string path = (spec.path / name).string();
            stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
            if (!data) {
                Log::Error(std::format("Failed to load cubemap texture at path: {}", path));
                continue;
            }
            
            if (!storageAllocated) {
                glTextureStorage2D(m_rendererID, 1, GL_RGBA8, width, height);
                storageAllocated = true;
            }
            
            glTextureSubImage3D(
                m_rendererID,
                0,
                0, 0, static_cast<int>(face),
                width, height, 1,
                GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        
        glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        m_width = width;
        m_height = height;
        m_isLoaded = true;
    } else {
        createRuntimeTexture(spec);
    }
}

OpenGLTextureCube::~OpenGLTextureCube() {
    glDeleteTextures(1, &m_rendererID);
}

void OpenGLTextureCube::bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_rendererID);
}

void OpenGLTextureCube::setData(void *data, uint32_t size) {
    FERMION_ASSERT(false, "OpenGLTextureCube::setData not implemented");
}

void OpenGLTextureCube::createRuntimeTexture(const TextureCubeSpecification &spec) {
    m_width = spec.width;
    m_height = spec.height;
    
    GLenum internalFormat = Utils::fermionImageFormatToGLInternalFormat(spec.format);
    GLenum dataFormat = Utils::fermionImageFormatToGLDataFormat(spec.format);
    uint32_t mipLevels = spec.generateMips ? spec.maxMipLevels : 1;
    

    // 使用传统API
    glGenTextures(1, &m_rendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);
    
    GLenum dataType = (spec.format == ImageFormat::RGB16F || spec.format == ImageFormat::RG16F || spec.format == ImageFormat::RGBA32F) ? GL_FLOAT : GL_UNSIGNED_BYTE;
    
    for (uint32_t mip = 0; mip < mipLevels; ++mip) {
        uint32_t mipWidth = std::max(1u, m_width >> mip);
        uint32_t mipHeight = std::max(1u, m_height >> mip);
        
        for (uint32_t face = 0; face < 6; ++face) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                mip,
                internalFormat,
                mipWidth, mipHeight,
                0,
                dataFormat,
                dataType,
                nullptr
            );
        }
    }
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Log::Error("OpenGL error during cubemap creation");
    }
    
    if (spec.generateMips) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
    } else {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    

    m_isLoaded = true;
}

void OpenGLTextureCube::copyFromFramebuffer(std::shared_ptr<Framebuffer> fb, uint32_t face, uint32_t mipLevel) {

    GLint currentActiveTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &currentActiveTexture);
    
    glActiveTexture(GL_TEXTURE31);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);
    
    GLenum cubeFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
    uint32_t mipWidth = m_width >> mipLevel;
    uint32_t mipHeight = m_height >> mipLevel;
    
    glCopyTexSubImage2D(
        cubeFace,
        mipLevel,
        0, 0,
        0, 0,
        mipWidth,
        mipHeight
    );
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Log::Error(std::format("    OpenGL error during copyFromFramebuffer: 0x{:X}", error));
    }
    
    // 恢复原来的活动纹理单元
    glActiveTexture(currentActiveTexture);
}

void OpenGLTextureCube::generateMipmaps() {
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void OpenGLTexture2D::copyFromFramebuffer(std::shared_ptr<Framebuffer> fb, uint32_t x, uint32_t y) {
    fb->bind();
    
    glBindTexture(GL_TEXTURE_2D, m_rendererID);
    
    glCopyTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        x, y,
        m_width,
        m_height
    );
    
    glBindTexture(GL_TEXTURE_2D, 0);
    fb->unbind();
}

} // namespace Fermion
