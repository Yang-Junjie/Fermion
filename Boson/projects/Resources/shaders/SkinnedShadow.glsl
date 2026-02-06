#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 4) in ivec4 a_BoneIDs;
layout(location = 5) in vec4 a_BoneWeights;

// Model uniform buffer (binding = 1)
layout(std140, binding = 1) uniform ModelData
{
	mat4 u_Model;
	mat4 u_NormalMatrix;
	int u_ObjectID;
};

// Light uniform buffer (binding = 2)
layout(std140, binding = 2) uniform LightData
{
	mat4 u_LightSpaceMatrix;
	vec3 u_DirLightDirection;
	float u_DirLightIntensity;
	vec3 u_DirLightColor;
	float _lightPadding0;
	float u_ShadowBias;
	float u_ShadowSoftness;
	int u_EnableShadows;
	float _lightPadding1;
	float u_AmbientIntensity;
	int u_NumPointLights;
	int u_NumSpotLights;
};

// Bone uniform buffer (binding = 4)
layout(std140, binding = 4) uniform BoneData
{
	mat4 u_BoneMatrices[128];
};

void main() {
    // Compute skin matrix from bone influences
    mat4 skinMatrix = mat4(0.0);
    for (int i = 0; i < 4; i++) {
        if (a_BoneIDs[i] >= 0) {
            skinMatrix += a_BoneWeights[i] * u_BoneMatrices[a_BoneIDs[i]];
        }
    }

    // If no bones affect this vertex, use identity
    if (a_BoneWeights[0] == 0.0 && a_BoneWeights[1] == 0.0 &&
        a_BoneWeights[2] == 0.0 && a_BoneWeights[3] == 0.0) {
        skinMatrix = mat4(1.0);
    }

    vec4 skinnedPos = skinMatrix * vec4(a_Position, 1.0);
    gl_Position = u_LightSpaceMatrix * u_Model * skinnedPos;
}

#type fragment
#version 450 core

void main() {

}
