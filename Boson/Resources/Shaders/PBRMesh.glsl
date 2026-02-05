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

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_Color;
out vec2 v_TexCoords;
out vec4 v_FragPosLightSpace;
out mat3 v_TBN;
flat out int v_ObjectID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    // Use precomputed normal matrix from UBO
    mat3 normalMatrix = mat3(u_NormalMatrix);
    v_Normal = normalize(normalMatrix * a_Normal);

    // Build TBN matrix for normal mapping
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 N = v_Normal;
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);

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
in mat3 v_TBN;
flat in int v_ObjectID;

const float PI = 3.14159265359;
const float HALF_PI = 1.570796327;
const float F0_NON_METAL = 0.04;
const float MIN_ROUGHNESS = 0.045;  // 避免高光过于尖锐

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

// 饱和函数
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

// 光源结构
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

// PBR材质参数
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

// ============================================================================
// 高级材质参数
// ============================================================================

// Clear Coat (透明涂层) - 用于汽车漆、涂漆表面等
uniform bool u_UseClearCoat;
uniform float u_ClearCoat;           // 涂层强度 [0, 1]
uniform float u_ClearCoatRoughness;  // 涂层粗糙度 [0, 1]

// Anisotropic (各向异性) - 用于拉丝金属、头发等
uniform bool u_UseAnisotropic;
uniform float u_Anisotropy;          // 各向异性强度 [-1, 1]，负值为垂直方向
uniform vec3 u_AnisotropyDirection;  // 各向异性方向（切线空间）

// Subsurface Scattering (次表面散射) - 用于皮肤、蜡、玉石等
uniform bool u_UseSubsurface;
uniform vec3 u_SubsurfaceColor;      // 次表面散射颜色
uniform float u_SubsurfacePower;     // 散射指数
uniform float u_Thickness;           // 材质厚度 [0, 1]，0=完全不透光

// Uniforms
uniform int u_DirLightCount;
uniform DirectionalLight u_DirLights[MAX_DIR_LIGHTS];

uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

uniform int u_SpotLightCount;
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

uniform Material u_Material;

// 纹理
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

// Shadow mapping
uniform sampler2D u_ShadowMap;

// IBL
uniform bool u_UseIBL;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLT;
uniform float u_PrefilterMaxLOD;


// GGX 法线分布函数 - 标准实现，使用 roughness² 参数化
// Walter et al. 2007, "Microfacet Models for Refraction through Rough Surfaces"
float D_GGX(float roughness, float NoH) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH2 = NoH * NoH;

    float nom = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 1e-7);
}

// Smith-GGX Correlated Visibility 函数 - 比 Schlick 近似更精确
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a = roughness * roughness;
    float a2 = a * a;
    float lambdaV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float lambdaL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / max(lambdaV + lambdaL, 1e-5);
}

// 快速近似版本 - 性能更好，质量略低
// Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
float V_SmithGGXCorrelated_Fast(float roughness, float NoV, float NoL) {
    float a = roughness * roughness;
    float v = 0.5 / max(mix(2.0 * NoL * NoV, NoL + NoV, a), 1e-5);
    return v;
}

// Schlick Fresnel - 使用优化的 pow5
vec3 F_Schlick(const vec3 f0, float f90, float VoH) {
    return f0 + (vec3(f90) - f0) * pow5(1.0 - VoH);
}

// Fresnel方程 - 标准版本，f90 = 1.0
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow5(saturate(1.0 - cosTheta));
}

// Fresnel方程带粗糙度 - 用于IBL
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow5(saturate(1.0 - cosTheta));
}

// Lambert 漫反射
float Fd_Lambert() {
    return 1.0 / PI;
}

// Burley 漫反射 - 比 Lambert 更真实，考虑粗糙度
// Burley 2012, "Physically-Based Shading at Disney"
float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoL);
    float viewScatter = 1.0 + (f90 - 1.0) * pow5(1.0 - NoV);
    return lightScatter * viewScatter * (1.0 / PI);
}

// ACES Filmic Tone Mapping - 更真实的色调映射
// Krzysztof Narkowicz 2015
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

// ============================================================================
// Clear Coat BRDF 函数
// 透明涂层使用简化的 Cook-Torrance，IOR 固定为 1.5 (聚氨酯)
// ============================================================================

// Clear Coat 专用的 Kelemen Visibility 函数 - 更高效
float V_Kelemen(float LoH) {
    return 0.25 / max(LoH * LoH, 1e-5);
}

// Clear Coat Fresnel - 固定 F0 = 0.04 (IOR 1.5)
float F_Schlick_ClearCoat(float VoH) {
    float f0 = 0.04;
    return f0 + (1.0 - f0) * pow5(1.0 - VoH);
}

// Clear Coat Lobe
float clearCoatLobe(float clearCoatRoughness, float NoH, float LoH, float clearCoat, out float Fcc) {
    float D = D_GGX(clearCoatRoughness, NoH);
    float V = V_Kelemen(LoH);
    float F = F_Schlick_ClearCoat(LoH) * clearCoat;

    Fcc = F;
    return D * V * F;
}

// ============================================================================
// Anisotropic BRDF 函数
// 各向异性高光 - 用于拉丝金属、头发、唱片等
// ============================================================================

// Anisotropic GGX NDF
// Burley 2012, "Physically-Based Shading at Disney"
float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d, d);
    float b2 = a2 / max(d2, 1e-7);
    return a2 * b2 * b2 * (1.0 / PI);
}

// Anisotropic Smith-GGX Visibility
float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
        float ToL, float BoL, float NoV, float NoL) {
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    return 0.5 / max(lambdaV + lambdaL, 1e-5);
}

// Anisotropic Specular Lobe
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

    // 计算各向异性粗糙度
    // Kulla 2017, "Revisiting Physically Based Shading at Imageworks"
    float at = max(roughness * (1.0 + anisotropy), MIN_ROUGHNESS);
    float ab = max(roughness * (1.0 - anisotropy), MIN_ROUGHNESS);

    float D = D_GGX_Anisotropic(at, ab, ToH, BoH, NoH);
    float V_term = V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
    vec3 F = fresnelSchlick(LoH, F0);

    kS = F;
    NoL_out = NoL;

    return (D * V_term) * F;
}

// ============================================================================
// Subsurface Scattering BRDF 函数
// 次表面散射 - 用于皮肤、蜡、玉石、树叶等半透明材质
// ============================================================================

// Wrap Diffuse - 能量守恒的包裹漫反射，模拟次表面散射
float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / sq(1.0 + w));
}

// Subsurface Scattering Lobe
// 基于 Filament 的简化 BTDF 实现
vec3 subsurfaceLobe(vec3 N, vec3 V, vec3 L, vec3 subsurfaceColor,
        float subsurfacePower, float thickness, vec3 diffuseColor) {

    float NoL = dot(N, L);

    // Forward scattering - 使用球面高斯近似
    // 模拟光线穿透材质后从背面散射出来
    float scatterVoH = saturate(dot(V, -L));
    float forwardScatter = exp2(scatterVoH * subsurfacePower - subsurfacePower);

    // Back scatter - 简化的背面散射
    float backScatter = saturate(NoL * thickness + (1.0 - thickness)) * 0.5;

    // 组合散射
    float subsurface = mix(backScatter, 1.0, forwardScatter) * (1.0 - thickness);

    return subsurfaceColor * subsurface * diffuseColor * (1.0 / PI);
}

// 阴影计算
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

// 获取法线使用导数计算TBN
vec3 getNormalFromMap(out float normalVariance) {
    if(!u_UseNormalMap) {
        normalVariance = 0.0;
        return normalize(v_Normal);
    }

    // 采样切线空间法线
    vec3 tangentNormal = texture(u_NormalMap, v_TexCoords).xyz * 2.0 - 1.0;

    // 构建 TBN（屏幕导数方式，避免切线错误）
    vec3 Q1 = dFdx(v_WorldPos);
    vec3 Q2 = dFdy(v_WorldPos);
    vec2 st1 = dFdx(v_TexCoords);
    vec2 st2 = dFdy(v_TexCoords);

    vec3 N = normalize(v_Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    // 世界空间法线
    vec3 worldNormal = normalize(TBN * tangentNormal);

    // 计算法线变化率（用于 Specular AA）
    vec3 dndx = dFdx(worldNormal);
    vec3 dndy = dFdy(worldNormal);
    normalVariance = max(0.0, dot(dndx, dndx) + dot(dndy, dndy));

    return worldNormal;
}

// Lambert diffuse - 使用优化版本
vec3 LambertDiffuse(vec3 kS, vec3 albedo, float metallic) {
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    return kD * albedo * Fd_Lambert();
}

// Burley diffuse - 更真实的漫反射
vec3 BurleyDiffuse(vec3 kS, vec3 albedo, float metallic, float roughness, float NoV, float NoL, float LoH) {
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    return kD * albedo * Fd_Burley(roughness, NoV, NoL, LoH);
}

// ============================================================================
// 综合着色函数 - 支持 Clear Coat, Anisotropic, Subsurface
// ============================================================================
vec3 evaluateLighting(vec3 N, vec3 V, vec3 L, vec3 T, vec3 B,
        vec3 albedo, float roughness, float metallic, vec3 F0,
        float NoV, vec3 radiance, float occlusion) {

    vec3 H = normalize(V + L);
    float NoL = max(dot(N, L), 0.0);
    float NoH = saturate(dot(N, H));
    float LoH = saturate(dot(L, H));

    vec3 color = vec3(0.0);

    // ==================== 基础 BRDF ====================
    vec3 kS;
    vec3 specularBRDF;

    if(u_UseAnisotropic && abs(u_Anisotropy) > 0.01) {
        // 各向异性高光
        float dummy;
        specularBRDF = anisotropicLobe(N, V, L, H, T, B, roughness, u_Anisotropy, F0, kS, dummy);
    } else {
        // 标准各向同性高光
        float D = D_GGX(roughness, NoH);
        float V_term = V_SmithGGXCorrelated(roughness, max(dot(N, V), 1e-4), NoL);
        kS = fresnelSchlick(LoH, F0);
        specularBRDF = (D * V_term) * kS;
    }

    // 漫反射
    vec3 diffuseBRDF = BurleyDiffuse(kS, albedo, metallic, roughness, NoV, NoL, LoH);

    // 次表面散射修正漫反射
    if(u_UseSubsurface) {
        // 使用 Wrap Diffuse 模拟次表面散射对漫反射的影响
        float wrapNoL = Fd_Wrap(dot(N, L), 0.5);
        diffuseBRDF *= saturate(u_SubsurfaceColor + wrapNoL);
    }

    color = (diffuseBRDF + specularBRDF) * NoL;

    // ==================== 次表面散射 ====================
    if(u_UseSubsurface) {
        vec3 sss = subsurfaceLobe(N, V, L, u_SubsurfaceColor,
            u_SubsurfacePower, u_Thickness, albedo);
        color += sss;
    }

    // ==================== Clear Coat ====================
    if(u_UseClearCoat && u_ClearCoat > 0.01) {
        // Clear Coat 使用几何法线，避免底层法线贴图细节影响涂层
        vec3 clearCoatNormal = normalize(v_Normal);
        float clearCoatNoH = saturate(dot(clearCoatNormal, H));
        float clearCoatNoL = saturate(dot(clearCoatNormal, L));

        float Fcc;
        float clearCoatSpecular = clearCoatLobe(
            max(u_ClearCoatRoughness, MIN_ROUGHNESS),
            clearCoatNoH, LoH, u_ClearCoat, Fcc);

        // Clear Coat 吸收底层能量
        float attenuation = 1.0 - Fcc;
        color *= attenuation;

        // 添加 Clear Coat 高光
        color += clearCoatSpecular * clearCoatNoL;
    }

    return color * radiance * occlusion;
}

// ============================================================================
// 主渲染函数
// ============================================================================

void main() {
    // 处理UV
    vec2 uv = v_TexCoords;
    if(u_FlipUV)
        uv.y = 1.0 - uv.y;

    // 获取材质属性
    vec3 baseColorLinear = pow(u_Material.albedo, vec3(2.2));
    vec3 texColorLinear = u_UseAlbedoMap ? pow(texture(u_AlbedoMap, uv).rgb, vec3(2.2)) : vec3(1.0);
    vec3 albedo = texColorLinear * baseColorLinear;
    float metallic = u_UseMetallicMap ? texture(u_MetallicMap, uv).r : u_Material.metallic;
    float roughness = u_UseRoughnessMap ? texture(u_RoughnessMap, uv).r : u_Material.roughness;
    float ao = u_UseAOMap ? texture(u_AOMap, uv).r : u_Material.ao;

    // 获取法线
    float normalVariance;
    vec3 N = getNormalFromMap(normalVariance);
    vec3 V = normalize(u_CameraPosition - v_WorldPos);
    float NoV = max(dot(N, V), 1e-4);

    // 构建 TBN 矩阵用于各向异性计算
    vec3 T = normalize(v_TBN[0]);
    vec3 B = normalize(v_TBN[1]);

    // 如果有自定义各向异性方向，应用它
    if(u_UseAnisotropic) {
        vec3 anisotropyDir = normalize(u_AnisotropyDirection);
        T = normalize(v_TBN * anisotropyDir);
        B = normalize(cross(N, T));
    }

    // 法线强度混合
    vec3 N_geom = normalize(v_Normal);
    N = normalize(mix(N_geom, N, u_NormalStrength));
    NoV = max(dot(N, V), 1e-4);

    // ================= Specular Anti-Aliasing =================
    float roughnessAA = clamp(roughness, 0.04, 1.0);
    if(u_UseNormalMap) {
        float variance = normalVariance;
        float kernelRoughness = min(2.0 * variance, 1.0);
        float normalLength = length(texture(u_NormalMap, uv).xyz * 2.0 - 1.0);
        float tokvsig = clamp((1.0 - normalLength), 0.0, 1.0);
        roughnessAA = clamp(sqrt(roughnessAA * roughnessAA + kernelRoughness + tokvsig * 0.5 * u_ToksvigStrength), 0.04, 1.0);
    }
    roughness = max(roughnessAA, MIN_ROUGHNESS);

    // 计算F0
    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    // 反射方程
    vec3 Lo = vec3(0.0);

    // ========================================================================
    // 主方向光（带阴影）
    // ========================================================================
    {
        vec3 L = normalize(-u_DirLightDirection);
        vec3 radiance = u_DirLightColor * u_DirLightIntensity;

        float shadow = 0.0;
        if(u_EnableShadows != 0) {
            shadow = calculateShadow(v_FragPosLightSpace, N, L);
        }

        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0 - shadow);
    }

    // ========================================================================
    // 额外方向光（无阴影）
    // ========================================================================
    for(int i = 0; i < u_DirLightCount; i++) {
        DirectionalLight light = u_DirLights[i];
        vec3 L = normalize(-light.direction);
        vec3 radiance = light.color * light.intensity;

        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // ========================================================================
    // 点光源
    // ========================================================================
    for(int i = 0; i < u_PointLightCount; i++) {
        PointLight light = u_PointLights[i];

        vec3 L = light.position - v_WorldPos;
        float lightDist = length(L);

        if(lightDist > light.range)
            continue;

        L = normalize(L);

        float distanceAttenuation = 1.0 / (lightDist * lightDist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(lightDist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;

        vec3 radiance = light.color * light.intensity * attenuation;

        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // ========================================================================
    // 聚光灯
    // ========================================================================
    for(int i = 0; i < u_SpotLightCount; i++) {
        SpotLight light = u_SpotLights[i];

        vec3 L = light.position - v_WorldPos;
        float lightDist = length(L);

        if(lightDist > light.range)
            continue;

        L = normalize(L);

        float theta = dot(L, normalize(light.direction));
        float epsilon = light.innerConeAngle - light.outerConeAngle;
        float spotIntensity = saturate((theta - light.outerConeAngle) / epsilon);

        if(spotIntensity <= 0.0)
            continue;

        float distanceAttenuation = 1.0 / (lightDist * lightDist + 1.0);
        float windowFactor = sq(saturate(1.0 - sq(sq(lightDist / light.range))));
        float attenuation = distanceAttenuation * windowFactor;

        vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

        Lo += evaluateLighting(N, V, L, T, B, albedo, roughness, metallic, F0, NoV, radiance, 1.0);
    }

    // ========================================================================
    // 环境光 (IBL)
    // ========================================================================
    vec3 ambient = vec3(0.0);

    if(u_UseIBL) {
        float NoV_ibl = max(dot(N, V), 1e-4);
        vec3 F = fresnelSchlickRoughness(NoV_ibl, F0, roughness);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        // 漫反射 IBL
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        if(dot(irradiance, irradiance) < 0.000001)
            irradiance = vec3(0.03);

        vec3 diffuse = irradiance * albedo;

        // 镜面反射 IBL
        vec3 R = reflect(-V, N);
        float lod = roughness * u_PrefilterMaxLOD;

        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, lod).rgb;
        if(dot(prefilteredColor, prefilteredColor) < 0.000001)
            prefilteredColor = vec3(0.03);

        vec2 brdf = texture(u_BRDFLT, vec2(NoV_ibl, roughness)).rg;
        float specOcclusion = computeSpecularOcclusion(NoV_ibl, ao, roughness);
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * specOcclusion;

        ambient = (kD * diffuse + specular) * ao;

        // Clear Coat IBL
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

    // HDR色调映射 (ACES Filmic)
    color = ACESFilm(color);
    // Gamma校正
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
    o_ObjectID = v_ObjectID;
}
