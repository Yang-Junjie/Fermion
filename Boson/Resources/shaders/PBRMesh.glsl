#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;
layout(location = 4) in vec3 a_Tangent;
layout(location = 5) in vec3 a_Bitangent;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_LightSpaceMatrix;
uniform int u_ObjectID;

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

    // 计算法线矩阵
    mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
    v_Normal = normalize(normalMatrix * a_Normal);

    // 构建TBN矩阵用于法线贴图
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 N = v_Normal;
    // 重新正交化T相对于N
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
#version 330 core

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
const float F0_NON_METAL = 0.04;

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

// 光源结构
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

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

// PBR材质参数
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

// Uniforms
uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

uniform int u_SpotLightCount;
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

uniform DirectionalLight u_DirectionalLight;

uniform Material u_Material;
uniform vec3 u_CameraPosition;
uniform float u_AmbientIntensity;

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
uniform bool u_EnableShadows;
uniform sampler2D u_ShadowMap;
uniform float u_ShadowBias;
uniform float u_ShadowSoftness;

// IBL
uniform bool u_UseIBL;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLT;
uniform float u_PrefilterMaxLOD;

// ============================================================================
// PBR函数
// ============================================================================

// 法线分布函数
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

// 几何遮蔽函数
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

// Fresnel方程
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel方程带粗糙度 - 用于IBL
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

// Lambert diffuse
vec3 LambertDiffuse(vec3 kS, vec3 albedo, float metallic) {
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    return (kD * albedo / PI);
}

// Cook-Torrance Specular
vec3 CookTorrance(vec3 N, vec3 L, vec3 V, float roughness, float metallic, vec3 F0, out vec3 kS) {
    vec3 H = normalize(V + L);

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    kS = F;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return (D * G * F) / (4.0 * max(NdotV * NdotL, 0.01));
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
    float NoV = max(dot(N, V), 0.0);

    // ==========================================================
    // Normal Map Weighting (Roughness-based Normal Filtering)
    // ==========================================================

    vec3 N_geom = normalize(v_Normal);

    float roughnessWeight = smoothstep(0.15, 0.75, 1.0 - roughness);
    float grazingAttenuation = smoothstep(0.0, 0.4, NoV);

    float normalWeight = roughnessWeight * grazingAttenuation;
    normalWeight *= u_NormalStrength;

    float distance = length(u_CameraPosition - v_WorldPos);
    float distanceBoost = clamp(1.5 - distance * 0.15, 0.7, 1.5);
    normalWeight *= distanceBoost;

    N = normalize(mix(N_geom, N, normalWeight));
    NoV = max(dot(N, V), 0.0);

   // ================= Specular Anti-Aliasing =================

// 法线变化导致的 roughness 提升（Karis 2013）
    float roughnessAA = clamp(roughness, 0.04, 1.0);
    if(u_UseNormalMap) {
        float variance = normalVariance;
        float kernelRoughness = min(2.0 * variance, 1.0);

    // Toksvig 修正（法线贴图能量损失）
        float normalLength = length(texture(u_NormalMap, uv).xyz * 2.0 - 1.0);
        float tokvsig = clamp((1.0 - normalLength), 0.0, 1.0);

        roughnessAA = clamp(sqrt(roughnessAA * roughnessAA + kernelRoughness + tokvsig * 0.5 * u_ToksvigStrength), 0.04, 1.0);
    }

    roughness = roughnessAA;

    // 计算F0
    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    // 反射方程
    vec3 Lo = vec3(0.0);

    // ========================================================================
    // 方向光
    // ========================================================================
    {
        vec3 L = normalize(-u_DirectionalLight.direction);

        vec3 kS;
        vec3 specularBRDF = CookTorrance(N, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(N, L), 0.0);
        vec3 radiance = u_DirectionalLight.color * u_DirectionalLight.intensity;

        // 计算阴影
        float shadow = 0.0;
        if(u_EnableShadows) {
            shadow = calculateShadow(v_FragPosLightSpace, N, L);
        }

        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL * (1.0 - shadow);
    }

    // ========================================================================
    // 点光源
    // ========================================================================
    for(int i = 0; i < u_PointLightCount; i++) {
        PointLight light = u_PointLights[i];

        vec3 L = light.position - v_WorldPos;
        float distance = length(L);

        if(distance > light.range)
            continue;

        L = normalize(L);

        // 衰减
        float attenuation = 1.0 - (distance / light.range);
        attenuation = attenuation * attenuation;

        vec3 radiance = light.color * light.intensity * attenuation;

        vec3 kS;
        vec3 specularBRDF = CookTorrance(N, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL;
    }

    // ========================================================================
    // 聚光灯
    // ========================================================================
    for(int i = 0; i < u_SpotLightCount; i++) {
        SpotLight light = u_SpotLights[i];

        vec3 L = light.position - v_WorldPos;
        float distance = length(L);

        if(distance > light.range)
            continue;

        L = normalize(L);

        // 聚光灯衰减
        float theta = dot(L, normalize(light.direction));
        float epsilon = light.innerConeAngle - light.outerConeAngle;
        float spotIntensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);

        if(spotIntensity <= 0.0)
            continue;

        // 距离衰减
        float attenuation = 1.0 - (distance / light.range);
        attenuation = attenuation * attenuation;

        vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

        vec3 kS;
        vec3 specularBRDF = CookTorrance(N, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL;
    }

    // ========================================================================
    // 环境光
    // ========================================================================
    vec3 ambient = vec3(0.0);

    if(u_UseIBL) {
        float NoV = max(dot(N, V), 0.0);

    // Fresnel
        vec3 F = fresnelSchlickRoughness(NoV, F0, roughness);

    // 能量守恒
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

    // ================= 漫反射 =================
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        if(length(irradiance) < 0.001)
            irradiance = vec3(0.03);

        vec3 diffuse = irradiance * albedo;

    // ================= 镜面反射 =================
        vec3 R = reflect(-V, N);

        float lod = roughness * u_PrefilterMaxLOD;

        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, lod).rgb;
        if(length(prefilteredColor) < 0.001)
            prefilteredColor = vec3(0.03);

        vec2 brdf = texture(u_BRDFLT, vec2(NoV, roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao;
    } else {
        // 简单环境光
        ambient = vec3(u_AmbientIntensity) * albedo * ao;
    }

    vec3 color = ambient + Lo;

    // HDR色调映射
    color = color / (color + vec3(1.0));
    // Gamma校正
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
    o_ObjectID = v_ObjectID;
}
