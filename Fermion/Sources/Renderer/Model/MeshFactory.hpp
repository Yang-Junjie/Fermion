#pragma once 
#include "Mesh.hpp"

namespace Fermion
{
    class MeshFactory
    {
    public:
        static AssetHandle CreateBox(const glm::vec3& size);
    }; 
}
