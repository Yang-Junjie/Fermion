#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;

// Camera uniform buffer (binding = 0)
layout(std140, binding = 0) uniform CameraData
{
	mat4 u_ViewProjection;
	mat4 u_View;
	mat4 u_Projection;
	vec3 u_CameraPosition;
};

// Model uniform buffer (binding = 1)
layout(std140, binding = 1) uniform ModelData
{
	mat4 u_Model;
	mat4 u_NormalMatrix;
	int u_ObjectID;
};

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoords;
flat out int v_ObjectID;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    // Use precomputed normal matrix from UBO
    mat3 normalMatrix = mat3(u_NormalMatrix);
    v_Normal = normalize(normalMatrix * a_Normal);

    v_TexCoords = a_TexCoords;
    v_ObjectID = u_ObjectID;

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Material;
layout(location = 3) out vec4 o_Emissive;
layout(location = 4) out int o_ObjectID;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoords;
flat in int v_ObjectID;

struct Material
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

uniform Material u_Material;

uniform bool u_UseAlbedoMap;
uniform sampler2D u_AlbedoMap;

uniform bool u_UseNormalMap;
uniform sampler2D u_NormalMap;
uniform float u_NormalStrength;
uniform float u_ToksvigStrength;

uniform bool u_UseMetallicMap;
uniform sampler2D u_MetallicMap;

uniform bool u_UseRoughnessMap;
uniform sampler2D u_RoughnessMap;

uniform bool u_UseAOMap;
uniform sampler2D u_AOMap;

uniform bool u_FlipUV;

vec3 getNormalFromMap(vec2 uv, out float normalVariance, out float normalLength)
{
    normalVariance = 0.0;
    normalLength = 1.0;

    if (!u_UseNormalMap)
        return normalize(v_Normal);

    vec3 tangentNormal = texture(u_NormalMap, uv).xyz * 2.0 - 1.0;
    normalLength = length(tangentNormal);

    vec3 Q1 = dFdx(v_WorldPos);
    vec3 Q2 = dFdy(v_WorldPos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 worldNormal = normalize(TBN * tangentNormal);
    worldNormal = normalize(mix(N, worldNormal, u_NormalStrength));

    vec3 dndx = dFdx(worldNormal);
    vec3 dndy = dFdy(worldNormal);
    normalVariance = max(0.0, dot(dndx, dndx) + dot(dndy, dndy));
    return worldNormal;
}

void main()
{
    vec2 uv = v_TexCoords;
    if (u_FlipUV)
        uv.y = 1.0 - uv.y;

    vec3 baseColorLinear = pow(u_Material.albedo, vec3(2.2));
    vec3 texColorLinear = u_UseAlbedoMap ? pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2)) : vec3(1.0);
    vec3 albedo = texColorLinear * baseColorLinear;

    float metallic = u_UseMetallicMap ? texture(u_MetallicMap, uv).r : u_Material.metallic;
    float roughness = u_UseRoughnessMap ? texture(u_RoughnessMap, uv).r : u_Material.roughness;
    float ao = u_UseAOMap ? texture(u_AOMap, uv).r : u_Material.ao;

    float normalVariance;
    float normalLength;
    vec3 normal = getNormalFromMap(uv, normalVariance, normalLength);

    float roughnessAA = clamp(roughness, 0.04, 1.0);
    if (u_UseNormalMap)
    {
        float kernelRoughness = min(2.0 * normalVariance, 1.0);
        float tokvsig = clamp(1.0 - normalLength, 0.0, 1.0);
        roughnessAA = clamp(sqrt(roughnessAA * roughnessAA + kernelRoughness + tokvsig * 0.5 * u_ToksvigStrength), 0.04, 1.0);
    }

    o_Albedo = vec4(albedo, 1.0);
    o_Normal = vec4(normalize(normal), 1.0);
    o_Material = vec4(roughnessAA, metallic, ao, 1.0);
    o_Emissive = vec4(0.0, 0.0, 0.0, 1.0);
    o_ObjectID = v_ObjectID;
}
