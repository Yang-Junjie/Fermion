#pragma once

#include <glm/glm.hpp>

namespace Fermion {

struct TextParams {
    glm::vec4 color{1.0f};
    float kerning = 0.0f;
    float lineSpacing = 0.0f;
};

} // namespace Fermion
