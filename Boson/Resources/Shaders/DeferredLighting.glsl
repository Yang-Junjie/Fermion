#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoords;

const float PI = 3.14159265359;
const float F0_NON_METAL = 0.04;
const float MIN_ROUGHNESS = 0.045;

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

// 优化的数学函数
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}

float sq(float x) {
    return x * x;
}

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

vec3 saturate(vec3 x) {
    return clamp(x, 0.0, 1.0);
}

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float range;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 color;
    float intensity;

    float range;
    float innerConeAngle;
    float outerConeAngle;
};

uniform int u_DirLightCount;
uniform DirectionalLight u_DirLights[MAX_DIR_LIGHTS];

uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

uniform int u_SpotLightCount;
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

// Camera uniform buffer (binding = 0)
layout(std140, binding = 0) uniform CameraData
{
	mat4 u_ViewProjection;
	mat4 u_View;
	mat4 u_Projection;
	vec3 u_CameraPosition;
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
	int u_NumDirLights;
	float u_AmbientIntensity;
	int u_NumPointLights;
	int u_NumSpotLights;
};

uniform sampler2D u_GBufferAlbedo;
uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferMaterial;
uniform sampler2D u_GBufferEmissive;
uniform sampler2D u_GBufferDepth;

uniform mat4 u_InverseViewProjection;

// Shadow mapping
uniform sampler2D u_ShadowMap;

// IBL
uniform bool u_UseIBL;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLT;
uniform float u_PrefilterMaxLOD;

// ============================================================================
// BRDF 函数
// ============================================================================

float D_GGX(float roughness, float NoH) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH2 = NoH * NoH;

    float nom = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 1e-7);
}

float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a = roughness * roughness;
    float a2 = a * a;
    float lambdaV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float lambdaL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / max(lambdaV + lambdaL, 1e-5);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow5(saturate(1.0 - cosTheta));
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow5(saturate(1.0 - cosTheta));
}

float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoL);
    float viewScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float computeSpecularOcclusion(float NoV, float ao, float roughness) {
    return clamp(pow(NoV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias * 0.1);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    int pcfRange = int(u_ShadowSoftness);
    int sampleCount = 0;

    for (int x = -pcfRange; x <= pcfRange; ++x) {
        for (int y = -pcfRange; y <= pcfRange; ++y) {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);

    return shadow;
}

vec3 reconstructWorldPosition(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 world = u_InverseViewProjection * clip;
    return world.xyz / max(world.w, 0.0001);
}

// ============================================================================
// 综合光照评估
// ============================================================================
vec3 evaluateLighting(vec3 N, vec3 V, vec3 L, vec3 albedo, float roughness,
        float metallic, vec3 F0, float NoV, vec3 radiance, float occlusion) {

    vec3 H = normalize(V + L);
    float NoL = max(dot(N, L), 0.0);
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));

    // 标准 BRDF
    float D = D_GGX(roughness, NoH);
    float V_term = V_SmithGGXCorrelated(roughness, NoV, NoL);
    vec3 kS = fresnelSchlick(LoH, F0);
    vec3 specularBRDF = (D * V_term) * kS;

    // Burley 漫反射
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuseBRDF = kD * albedo * Fd_Burley(roughness, NoV, NoL, LoH);

    vec3 color = (diffuseBRDF + specularBRDF) * NoL;

    return color * radiance * occlusion;
}

// ============================================================================
// 主渲染函数
// ============================================================================
void main() {
    float depth = texture(u_GBufferDepth, v_TexCoords).r;
    if (depth >= 1.0)
        discard;

    vec3 albedo = texture(u_GBufferAlbedo, v_TexCoords).rgb;
    vec3 N = normalize(texture(u_GBufferNormal, v_TexCoords).rgb);
    vec3 material = texture(u_GBufferMaterial, v_TexCoords).rgb;
    vec3 emissive = texture(u_GBufferEmissive, v_TexCoords).rgb;

    float roughness = max(material.r, MIN_ROUGHNESS);
    float metallic = saturate(material.g);
    float ao = saturate(material.b);

    vec3 worldPos = reconstructWorldPosition(v_TexCoords, depth);
    vec3 V = normalize(u_CameraPosition - worldPos);
    float NoV = max(dot(N, V), 1e-4);

    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Main directional light (with shadow)
    {
        vec3 L = normalize(-u_DirLightDirection);
        vec3 radiance = u_DirLightColor * u_DirLightIntensity;

        float shadow = 0.0;
        if (u_EnableShadows != 0) {
            vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(worldPos, 1.0);
            shadow = calculateShadow(fragPosLightSpace, N, L);
        }

        Lo += evaluateLighting(N, V, L, albedo, roughness, metallic, F0, NoV, radiance, 1.0 - shadow);
    }

    // Additional directional lights
    for (int i = 0; i < u_DirLightCount; i++) {
        DirectionalLight light = u_DirLights[i];
        vec3 L = normalize(-light.direction);
        vec3 radiance = light.color * light.intensity;
        Lo += evaluateLighting(N, V, L, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // Point lights
    for (int i = 0; i < u_PointLightCount; i++) {
        PointLight light = u_PointLights[i];
        vec3 L = light.position - worldPos;
        float dist = length(L);
        if (dist > light.range)
            continue;

        L = normalize(L);
        float distanceAttenuation = 1.0 / (dist * dist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(dist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;
        vec3 radiance = light.color * light.intensity * attenuation;

        Lo += evaluateLighting(N, V, L, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // Spot lights
    for (int i = 0; i < u_SpotLightCount; i++) {
        SpotLight light = u_SpotLights[i];
        vec3 L = light.position - worldPos;
        float dist = length(L);
        if (dist > light.range)
            continue;

        L = normalize(L);
        float theta = dot(L, normalize(light.direction));
        float epsilon = light.innerConeAngle - light.outerConeAngle;
        float spotIntensity = saturate((theta - light.outerConeAngle) / epsilon);
        if (spotIntensity <= 0.0)
            continue;

        float distanceAttenuation = 1.0 / (dist * dist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(dist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;
        vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

        Lo += evaluateLighting(N, V, L, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // IBL
    vec3 ambient = vec3(0.0);
    if (u_UseIBL) {
        vec3 F = fresnelSchlickRoughness(NoV, F0, roughness);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        if (dot(irradiance, irradiance) < 0.000001)
            irradiance = vec3(0.03);
        vec3 diffuse = irradiance * albedo;

        vec3 R = reflect(-V, N);
        float lod = roughness * u_PrefilterMaxLOD;
        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, lod).rgb;
        if (dot(prefilteredColor, prefilteredColor) < 0.000001)
            prefilteredColor = vec3(0.03);

        vec2 brdf = texture(u_BRDFLT, vec2(NoV, roughness)).rg;
        float specOcclusion = computeSpecularOcclusion(NoV, ao, roughness);
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * specOcclusion;

        ambient = (kD * diffuse + specular) * ao;
    } else {
        ambient = vec3(u_AmbientIntensity) * albedo * ao;
    }

    vec3 color = ambient + Lo + emissive;

    color = ACESFilm(color);
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}
