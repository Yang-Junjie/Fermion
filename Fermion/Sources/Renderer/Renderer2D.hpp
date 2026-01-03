#pragma once
#include "Texture/Texture.hpp"
#include "Texture/SubTexture2D.hpp"
#include "Camera/OrthographicCameraController.hpp"
#include "Camera/Camera.hpp"
#include "Camera/EditorCamera.hpp"
#include "Font/Font.hpp"
#include "RendererConfig.hpp"
#include "Math/AABB.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/CommandBuffer.hpp"

namespace Fermion {
    class Renderer2D {
    public:
        static void init(const RendererConfig &config);

        static void shutdown();

        static void beginScene(const OrthographicCamera &camera);

        static void beginScene(const EditorCamera &camera);

        static void beginScene(const Camera &camera, const glm::mat4 &Transform);

        static void endScene();

        static void flush();

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size,
                             const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                             glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size,
                             const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                             glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size,
                             const std::shared_ptr<SubTexture2D> &subTexture, float tilingFactor = 1.0f,
                             glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size,
                             const std::shared_ptr<SubTexture2D> &subTexture, float tilingFactor = 1.0f,
                             glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawQuad(const glm::mat4 &transform, const glm::vec4 &color, int objectId = -1);

        static void drawQuad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture,
                             float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f), int objectId = -1);

        static void drawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color,
                                      int objectId = -1);

        static void drawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size,
                                      const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                                      const glm::vec4 &tintColor = glm::vec4(1.0f), int objectId = -1);
        
        static void drawAABB(const AABB& aabb,const glm::mat4& transform,const glm::vec4& color, int objectId = -1);

        static void drawQuad(const glm::mat4 &transform, const std::shared_ptr<SubTexture2D> &subTexture,
                             float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawQuadInstanced(const glm::mat4 &transform, const glm::vec4 &color,
                                      const std::shared_ptr<Texture2D> &texture, float tilingFactor, int objectID);

        static void drawQuadInstanced(const glm::mat4 &transform, const glm::vec4 &color, int objectID = -1);

        static void drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                    const glm::vec4 &color);

        static void drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                    const glm::vec4 &color);

        static void drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                    const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                                    glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                    const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                                    glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                    const std::shared_ptr<SubTexture2D> &subTexture, float tilingFactor = 1.0f,
                                    glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                    const std::shared_ptr<SubTexture2D> &subTexture, float tilingFactor = 1.0f,
                                    glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness = 1.0f,
                               float fade = 0.005f, int objectId = -1);

        static void drawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color, int objectId = -1);

        static void drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color,
                             int objectId = -1);

        static void drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId = -1);

        static float getLineWidth();

        static void setLineWidth(float width);
        
        static void recordOutlinePass(CommandBuffer &commandBuffer, const std::vector<MeshDrawCommand> &drawCommands);

        struct TextParams {
            glm::vec4 color{1.0f};
            float kerning = 0.0f;
            float lineSpacing = 0.0f;
        };

        static void drawString(const std::string &string, std::shared_ptr<Font> font, const glm::mat4 &transform,
                               const TextParams &textParams, int objectId = -1);

        struct Satistics {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;
            uint32_t lineCount = 0;
            uint32_t circleCount = 0;

            uint32_t getTotalVertexCount() {
                return quadCount * 4 + lineCount * 2 + circleCount * 4;
            }

            uint32_t getTotalIndexCount() {
                return quadCount * 6 + circleCount * 6;
            }
        };

        static void resetStatistics();

        static Satistics getStatistics();

    private:
        static void flushAndReset();
    };
} // namespace Fermion
