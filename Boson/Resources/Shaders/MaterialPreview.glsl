#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;
uniform mat4 u_NormalMatrix;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoords;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    mat3 normalMatrix = mat3(u_NormalMatrix);
    v_Normal = normalize(normalMatrix * a_Normal);

    v_TexCoords = a_TexCoords;

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoords;

const float PI = 3.14159265359;
const float F0_NON_METAL = 0.04;

// Camera
uniform vec3 u_CameraPosition;

// Light
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform float u_AmbientIntensity;

// PBR Material
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

uniform Material u_Material;

// Textures
uniform bool u_UseAlbedoMap;
uniform sampler2D u_AlbedoMap;

uniform bool u_UseNormalMap;
uniform sampler2D u_NormalMap;

uniform bool u_UseMetallicMap;
uniform sampler2D u_MetallicMap;

uniform bool u_UseRoughnessMap;
uniform sampler2D u_RoughnessMap;

uniform bool u_UseAOMap;
uniform sampler2D u_AOMap;

// ============================================================================
// PBR Functions
// ============================================================================

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith's method
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Get normal from normal map using screen-space derivatives
vec3 getNormalFromMap() {
    if (!u_UseNormalMap) {
        return normalize(v_Normal);
    }

    vec3 tangentNormal = texture(u_NormalMap, v_TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(v_WorldPos);
    vec3 Q2 = dFdy(v_WorldPos);
    vec2 st1 = dFdx(v_TexCoords);
    vec2 st2 = dFdy(v_TexCoords);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    // Get material properties
    vec3 baseColorLinear = pow(u_Material.albedo, vec3(2.2));
    vec3 texColorLinear = u_UseAlbedoMap ? pow(texture(u_AlbedoMap, v_TexCoords).rgb, vec3(2.2)) : vec3(1.0);
    vec3 albedo = texColorLinear * baseColorLinear;

    float metallic = u_UseMetallicMap ? texture(u_MetallicMap, v_TexCoords).r : u_Material.metallic;
    float roughness = u_UseRoughnessMap ? texture(u_RoughnessMap, v_TexCoords).r : u_Material.roughness;
    float ao = u_UseAOMap ? texture(u_AOMap, v_TexCoords).r : u_Material.ao;

    // Clamp roughness to avoid division by zero
    roughness = clamp(roughness, 0.04, 1.0);

    // Get normal
    vec3 N = getNormalFromMap();
    vec3 V = normalize(u_CameraPosition - v_WorldPos);

    // Calculate F0
    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    // Reflectance equation
    vec3 Lo = vec3(0.0);

    // Main directional light
    {
        vec3 L = normalize(-u_LightDirection);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);

        // Cook-Torrance BRDF
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = D * G * F;
        float denominator = 4.0 * max(NdotV * NdotL, 0.01);
        vec3 specular = numerator / denominator;

        // Energy conservation
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        vec3 radiance = u_LightColor * u_LightIntensity;
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting (simple)
    vec3 ambient = vec3(u_AmbientIntensity) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tone mapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}
