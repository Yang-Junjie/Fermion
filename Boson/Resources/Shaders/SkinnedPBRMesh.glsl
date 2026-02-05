#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in ivec4 a_BoneIDs;
layout(location = 5) in vec4 a_BoneWeights;

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

// Bone uniform buffer (binding = 4)
layout(std140, binding = 4) uniform BoneData
{
	mat4 u_BoneMatrices[128];
};

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_Color;
out vec2 v_TexCoords;
out vec4 v_FragPosLightSpace;
flat out int v_ObjectID;

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
    vec4 worldPos = u_Model * skinnedPos;
    v_WorldPos = worldPos.xyz;

    // Skin the normal
    mat3 skinNormalMatrix = mat3(skinMatrix);
    mat3 normalMatrix = mat3(u_NormalMatrix);
    v_Normal = normalize(normalMatrix * skinNormalMatrix * a_Normal);

    v_Color = a_Color;
    v_TexCoords = a_TexCoords;
    v_FragPosLightSpace = u_LightSpaceMatrix * worldPos;
    v_ObjectID = u_ObjectID;

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec4 v_Color;
in vec2 v_TexCoords;
in vec4 v_FragPosLightSpace;
flat in int v_ObjectID;

const float PI = 3.14159265359;
const float HALF_PI = 1.570796327;
const float F0_NON_METAL = 0.04;
const float MIN_ROUGHNESS = 0.045;

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

#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

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

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

// Clear Coat
uniform bool u_UseClearCoat;
uniform float u_ClearCoat;
uniform float u_ClearCoatRoughness;

// Anisotropic
uniform bool u_UseAnisotropic;
uniform float u_Anisotropy;
uniform vec3 u_AnisotropyDirection;

// Subsurface Scattering
uniform bool u_UseSubsurface;
uniform vec3 u_SubsurfaceColor;
uniform float u_SubsurfacePower;
uniform float u_Thickness;

uniform int u_DirLightCount;
uniform DirectionalLight u_DirLights[MAX_DIR_LIGHTS];
uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform int u_SpotLightCount;
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

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
uniform sampler2D u_ShadowMap;

// IBL
uniform bool u_UseIBL;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLT;
uniform float u_PrefilterMaxLOD;

// GGX NDF
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

vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    return f0 + (vec3(f90) - f0) * pow5(1.0 - VoH);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow5(saturate(1.0 - cosTheta));
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow5(saturate(1.0 - cosTheta));
}

float Fd_Lambert() {
    return 1.0 / PI;
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

float V_Kelemen(float LoH) {
    return 0.25 / max(LoH * LoH, 1e-5);
}

float F_Schlick_ClearCoat(float VoH) {
    float f0 = 0.04;
    return f0 + (1.0 - f0) * pow5(1.0 - VoH);
}

float clearCoatLobe(float clearCoatRoughness, float NoH, float LoH, float clearCoat, out float Fcc) {
    float D = D_GGX(clearCoatRoughness, NoH);
    float V = V_Kelemen(LoH);
    float F = F_Schlick_ClearCoat(LoH) * clearCoat;
    Fcc = F;
    return D * V * F;
}

float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d_vec = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d_vec, d_vec);
    float b2 = a2 / max(d2, 1e-7);
    return a2 * b2 * b2 * (1.0 / PI);
}

float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
        float ToL, float BoL, float NoV, float NoL) {
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    return 0.5 / max(lambdaV + lambdaL, 1e-5);
}

vec3 anisotropicLobe(vec3 N, vec3 V, vec3 L, vec3 H, vec3 T, vec3 B,
        float roughness, float anisotropy, vec3 F0, out vec3 kS, out float NoL_out) {
    float NoV = max(dot(N, V), 1e-4);
    float NoL = max(dot(N, L), 0.0);
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));
    float ToV = dot(T, V);
    float BoV = dot(B, V);
    float ToL = dot(T, L);
    float BoL = dot(B, L);
    float ToH = dot(T, H);
    float BoH = dot(B, H);
    float at = max(roughness * (1.0 + anisotropy), MIN_ROUGHNESS);
    float ab = max(roughness * (1.0 - anisotropy), MIN_ROUGHNESS);
    float D = D_GGX_Anisotropic(at, ab, ToH, BoH, NoH);
    float V_term = V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
    vec3 F = fresnelSchlick(LoH, F0);
    kS = F;
    NoL_out = NoL;
    return (D * V_term) * F;
}

float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / sq(1.0 + w));
}

vec3 subsurfaceLobe(vec3 N, vec3 V, vec3 L, vec3 subsurfaceColor,
        float subsurfacePower, float thickness, vec3 diffuseColor) {
    float NoL = dot(N, L);
    float scatterVoH = saturate(dot(V, -L));
    float forwardScatter = exp2(scatterVoH * subsurfacePower - subsurfacePower);
    float backScatter = saturate(NoL * thickness + (1.0 - thickness)) * 0.5;
    float subsurface = mix(backScatter, 1.0, forwardScatter) * (1.0 - thickness);
    return subsurfaceColor * subsurface * diffuseColor * (1.0 / PI);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias * 0.1);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    int pcfRange = int(u_ShadowSoftness);
    int sampleCount = 0;
    for(int x = -pcfRange; x <= pcfRange; ++x) {
        for(int y = -pcfRange; y <= pcfRange; ++y) {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);
    return shadow;
}

vec3 getNormalFromMap(out float normalVariance) {
    if(!u_UseNormalMap) {
        normalVariance = 0.0;
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
    vec3 worldNormal = normalize(TBN * tangentNormal);
    vec3 dndx = dFdx(worldNormal);
    vec3 dndy = dFdy(worldNormal);
    normalVariance = max(0.0, dot(dndx, dndx) + dot(dndy, dndy));
    return worldNormal;
}

vec3 BurleyDiffuse(vec3 kS, vec3 albedo, float metallic, float roughness, float NoV, float NoL, float LoH) {
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    return kD * albedo * Fd_Burley(roughness, NoV, NoL, LoH);
}

vec3 evaluateLighting(vec3 N, vec3 V, vec3 L, vec3 T, vec3 B,
        vec3 albedo, float roughness, float metallic, vec3 F0,
        float NoV, vec3 radiance, float occlusion) {
    vec3 H = normalize(V + L);
    float NoL = max(dot(N, L), 0.0);
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));
    vec3 color = vec3(0.0);
    vec3 kS;
    vec3 specularBRDF;
    if(u_UseAnisotropic && abs(u_Anisotropy) > 0.01) {
        float dummy;
        specularBRDF = anisotropicLobe(N, V, L, H, T, B, roughness, u_Anisotropy, F0, kS, dummy);
    } else {
        float D = D_GGX(roughness, NoH);
        float V_term = V_SmithGGXCorrelated(roughness, max(dot(N, V), 1e-4), NoL);
        kS = fresnelSchlick(LoH, F0);
        specularBRDF = (D * V_term) * kS;
    }
    vec3 diffuseBRDF = BurleyDiffuse(kS, albedo, metallic, roughness, NoV, NoL, LoH);
    if(u_UseSubsurface) {
        float wrapNoL = Fd_Wrap(dot(N, L), 0.5);
        diffuseBRDF *= saturate(u_SubsurfaceColor + wrapNoL);
    }
    color = (diffuseBRDF + specularBRDF) * NoL;
    if(u_UseSubsurface) {
        vec3 sss = subsurfaceLobe(N, V, L, u_SubsurfaceColor,
            u_SubsurfacePower, u_Thickness, albedo);
        color += sss;
    }
    if(u_UseClearCoat && u_ClearCoat > 0.01) {
        vec3 clearCoatNormal = normalize(v_Normal);
        float clearCoatNoH = saturate(dot(clearCoatNormal, H));
        float clearCoatNoL = saturate(dot(clearCoatNormal, L));
        float Fcc;
        float clearCoatSpecular = clearCoatLobe(
            max(u_ClearCoatRoughness, MIN_ROUGHNESS),
            clearCoatNoH, LoH, u_ClearCoat, Fcc);
        float attenuation = 1.0 - Fcc;
        color *= attenuation;
        color += clearCoatSpecular * clearCoatNoL;
    }
    return color * radiance * occlusion;
}

void main() {
    vec2 uv = v_TexCoords;
    if(u_FlipUV)
        uv.y = 1.0 - uv.y;
    vec3 baseColorLinear = pow(u_Material.albedo, vec3(2.2));
    vec3 texColorLinear = u_UseAlbedoMap ? pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2)) : vec3(1.0);
    vec3 albedo = texColorLinear * baseColorLinear;
    float metallic = u_UseMetallicMap ? texture(u_MetallicMap, uv).r : u_Material.metallic;
    float roughness = u_UseRoughnessMap ? texture(u_RoughnessMap, uv).r : u_Material.roughness;
    float ao = u_UseAOMap ? texture(u_AOMap, uv).r : u_Material.ao;
    float normalVariance;
    vec3 N = getNormalFromMap(normalVariance);
    vec3 V = normalize(u_CameraPosition - v_WorldPos);
    float NoV = max(dot(N, V), 1e-4);

    // Compute T and B using screen-space derivatives (same as getNormalFromMap)
    vec3 Q1 = dFdx(v_WorldPos);
    vec3 Q2 = dFdy(v_WorldPos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));

    if(u_UseAnisotropic) {
        vec3 anisotropyDir = normalize(u_AnisotropyDirection);
        vec3 N_geom = normalize(v_Normal);
        mat3 TBN = mat3(T, B, N_geom);
        T = normalize(TBN * anisotropyDir);
        B = normalize(cross(N, T));
    }
    vec3 N_geom = normalize(v_Normal);
    float roughnessWeight = smoothstep(0.15, 0.75, 1.0 - roughness);
    float grazingAttenuation = smoothstep(0.0, 0.4, NoV);
    float normalWeight = roughnessWeight * grazingAttenuation;
    normalWeight *= u_NormalStrength;
    float dist = length(u_CameraPosition - v_WorldPos);
    float distanceBoost = clamp(1.5 - dist * 0.15, 0.7, 1.5);
    normalWeight *= distanceBoost;
    N = normalize(mix(N_geom, N, normalWeight));
    NoV = max(dot(N, V), 1e-4);
    float roughnessAA = clamp(roughness, 0.04, 1.0);
    if(u_UseNormalMap) {
        float variance = normalVariance;
        float kernelRoughness = min(2.0 * variance, 1.0);
        float normalLength = length(texture(u_NormalMap, uv).xyz * 2.0 - 1.0);
        float tokvsig = clamp((1.0 - normalLength), 0.0, 1.0);
        roughnessAA = clamp(sqrt(roughnessAA * roughnessAA + kernelRoughness + tokvsig * 0.5 * u_ToksvigStrength), 0.04, 1.0);
    }
    roughness = max(roughnessAA, MIN_ROUGHNESS);
    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);
    vec3 Lo = vec3(0.0);
    {
        vec3 L = normalize(-u_DirLightDirection);
        vec3 radiance = u_DirLightColor * u_DirLightIntensity;
        float shadow = 0.0;
        if(u_EnableShadows != 0) {
            shadow = calculateShadow(v_FragPosLightSpace, N, L);
        }
        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0 - shadow);
    }
    for(int i = 0; i < u_DirLightCount; i++) {
        DirectionalLight light = u_DirLights[i];
        vec3 L = normalize(-light.direction);
        vec3 radiance = light.color * light.intensity;
        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }
    for(int i = 0; i < u_PointLightCount; i++) {
        PointLight light = u_PointLights[i];
        vec3 L = light.position - v_WorldPos;
        float lightDist = length(L);
        if(lightDist > light.range) continue;
        L = normalize(L);
        float distanceAttenuation = 1.0 / (lightDist * lightDist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(lightDist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;
        vec3 radiance = light.color * light.intensity * attenuation;
        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }
    for(int i = 0; i < u_SpotLightCount; i++) {
        SpotLight light = u_SpotLights[i];
        vec3 L = light.position - v_WorldPos;
        float lightDist = length(L);
        if(lightDist > light.range) continue;
        L = normalize(L);
        float theta = dot(L, normalize(light.direction));
        float epsilon = light.innerConeAngle - light.outerConeAngle;
        float spotIntensity = saturate((theta - light.outerConeAngle) / epsilon);
        if(spotIntensity <= 0.0) continue;
        float distanceAttenuation = 1.0 / (lightDist * lightDist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(lightDist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;
        vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;
        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }
    vec3 ambient = vec3(0.0);
    if(u_UseIBL) {
        float NoV_ibl = max(dot(N, V), 1e-4);
        vec3 F = fresnelSchlickRoughness(NoV_ibl, F0, roughness);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        if(dot(irradiance, irradiance) < 0.000001) irradiance = vec3(0.03);
        vec3 diffuse = irradiance * albedo;
        vec3 R = reflect(-V, N);
        float lod = roughness * u_PrefilterMaxLOD;
        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, lod).rgb;
        if(dot(prefilteredColor, prefilteredColor) < 0.000001) prefilteredColor = vec3(0.03);
        vec2 brdf = texture(u_BRDFLT, vec2(NoV_ibl, roughness)).rg;
        float specOcclusion = computeSpecularOcclusion(NoV_ibl, ao, roughness);
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * specOcclusion;
        ambient = (kD * diffuse + specular) * ao;
        if(u_UseClearCoat && u_ClearCoat > 0.01) {
            vec3 clearCoatR = reflect(-V, normalize(v_Normal));
            float clearCoatLod = u_ClearCoatRoughness * u_PrefilterMaxLOD;
            vec3 clearCoatPrefilteredColor = textureLod(u_PrefilterMap, clearCoatR, clearCoatLod).rgb;
            float Fc = F_Schlick_ClearCoat(NoV_ibl) * u_ClearCoat;
            ambient *= (1.0 - Fc);
            ambient += clearCoatPrefilteredColor * Fc;
        }
    } else {
        ambient = vec3(u_AmbientIntensity) * albedo * ao;
    }
    vec3 color = ambient + Lo;
    color = ACESFilm(color);
    color = pow(color, vec3(1.0 / 2.2));
    o_Color = vec4(color, 1.0);
    o_ObjectID = v_ObjectID;
}
