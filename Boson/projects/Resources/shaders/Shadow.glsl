#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

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

void main() {
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

void main() {

}