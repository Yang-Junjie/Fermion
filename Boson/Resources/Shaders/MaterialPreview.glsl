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
const float MIN_ROUGHNESS = 0.045;

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
// Utility Functions
// ============================================================================

float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

vec3 saturate(vec3 x) {
    return clamp(x, 0.0, 1.0);
}

// ============================================================================
// PBR Functions
// ============================================================================

// Normal Distribution Function (GGX)
float D_GGX(float roughness, float NoH) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH2 = NoH * NoH;

    float nom = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 1e-7);
}

// Smith-GGX Correlated Visibility
float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a = roughness * roughness;
    float a2 = a * a;
    float lambdaV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float lambdaL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / max(lambdaV + lambdaL, 1e-5);
}

// Fresnel-Schlick
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow5(saturate(1.0 - cosTheta));
}

// Burley Diffuse
float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoL);
    float viewScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoV);
    return lightScatter * viewScatter * (1.0 / PI);
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

// ACES Filmic Tone Mapping
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

void main() {
    // Get material properties
    vec3 baseColorLinear = pow(u_Material.albedo, vec3(2.2));
    vec3 texColorLinear = u_UseAlbedoMap ? pow(texture(u_AlbedoMap, v_TexCoords).rgb, vec3(2.2)) : vec3(1.0);
    vec3 albedo = texColorLinear * baseColorLinear;

    float metallic = u_UseMetallicMap ? texture(u_MetallicMap, v_TexCoords).r : u_Material.metallic;
    float roughness = u_UseRoughnessMap ? texture(u_RoughnessMap, v_TexCoords).r : u_Material.roughness;
    float ao = u_UseAOMap ? texture(u_AOMap, v_TexCoords).r : u_Material.ao;

    // Clamp roughness
    roughness = max(roughness, MIN_ROUGHNESS);

    // Get normal
    vec3 N = getNormalFromMap();
    vec3 V = normalize(u_CameraPosition - v_WorldPos);
    float NoV = max(dot(N, V), 1e-4);

    // Calculate F0
    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    // Reflectance equation
    vec3 Lo = vec3(0.0);

    // Main directional light
    {
        vec3 L = normalize(-u_LightDirection);
        vec3 H = normalize(V + L);

        float NoL = max(dot(N, L), 0.0);
        float NoH = saturate(dot(N, H));
        float LoH = saturate(dot(L, H));

        // Cook-Torrance BRDF
        float D = D_GGX(roughness, NoH);
        float V_term = V_SmithGGXCorrelated(roughness, NoV, NoL);
        vec3 F = fresnelSchlick(LoH, F0);

        vec3 specularBRDF = (D * V_term) * F;

        // Burley diffuse
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        vec3 diffuseBRDF = kD * albedo * Fd_Burley(roughness, NoV, NoL, LoH);

        vec3 radiance = u_LightColor * u_LightIntensity;
        Lo += (diffuseBRDF + specularBRDF) * radiance * NoL;
    }

    // Ambient lighting
    vec3 ambient = vec3(u_AmbientIntensity) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tone mapping (ACES Filmic)
    color = ACESFilm(color);

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}
