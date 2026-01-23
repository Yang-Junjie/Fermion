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

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

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
	float _lightPadding1;
	float u_AmbientIntensity;
	int u_NumPointLights;
	int u_NumSpotLights;
};

uniform sampler2D u_GBufferAlbedo;
uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferMaterial;
uniform sampler2D u_GBufferEmissive;
uniform sampler2D u_GBufferDepth;
uniform sampler2D u_SSGI;
uniform sampler2D u_GTAO;

uniform mat4 u_InverseViewProjection;

// Shadow mapping
uniform sampler2D u_ShadowMap;

// IBL
uniform bool u_UseIBL;
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLT;
uniform float u_PrefilterMaxLOD;
uniform bool u_EnableSSGI;
uniform bool u_EnableGTAO;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float computeSpecularOcclusion(float NoV, float ao, float roughness)
{
    return clamp(pow(NoV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias * 0.1);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    int pcfRange = int(u_ShadowSoftness);
    int sampleCount = 0;

    for (int x = -pcfRange; x <= pcfRange; ++x)
    {
        for (int y = -pcfRange; y <= pcfRange; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);

    return shadow;
}

vec3 LambertDiffuse(vec3 kS, vec3 albedo, float metallic)
{
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    return (kD * albedo / PI);
}

vec3 CookTorrance(vec3 N, vec3 L, vec3 V, float roughness, float metallic, vec3 F0, out vec3 kS)
{
    vec3 H = normalize(V + L);

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    kS = F;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return (D * G * F) / (4.0 * max(NdotV * NdotL, 0.01));
}

vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 world = u_InverseViewProjection * clip;
    return world.xyz / max(world.w, 0.0001);
}

void main()
{
    float depth = texture(u_GBufferDepth, v_TexCoords).r;
    if (depth >= 1.0)
        discard;

    vec3 albedo = texture(u_GBufferAlbedo, v_TexCoords).rgb;
    vec3 normal = normalize(texture(u_GBufferNormal, v_TexCoords).rgb);
    vec3 material = texture(u_GBufferMaterial, v_TexCoords).rgb;
    vec3 emissive = texture(u_GBufferEmissive, v_TexCoords).rgb;

    float roughness = clamp(material.r, 0.04, 1.0);
    float metallic = clamp(material.g, 0.0, 1.0);
    float ao = clamp(material.b, 0.0, 1.0);
    float gtao = 1.0;
    if (u_EnableGTAO)
        gtao = clamp(texture(u_GTAO, v_TexCoords).r, 0.0, 1.0);
    float combinedAO = ao * gtao;

    vec3 worldPos = reconstructWorldPosition(v_TexCoords, depth);
    vec3 V = normalize(u_CameraPosition - worldPos);

    vec3 F0 = mix(vec3(F0_NON_METAL), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Directional light
    {
        vec3 L = normalize(-u_DirLightDirection);
        vec3 kS;
        vec3 specularBRDF = CookTorrance(normal, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(normal, L), 0.0);
        vec3 radiance = u_DirLightColor * u_DirLightIntensity;

        float shadow = 0.0;
        if (u_EnableShadows != 0)
        {
            vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(worldPos, 1.0);
            shadow = calculateShadow(fragPosLightSpace, normal, L);
        }

        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL * (1.0 - shadow);
    }

    // Point lights
    for (int i = 0; i < u_PointLightCount; i++)
    {
        PointLight light = u_PointLights[i];

        vec3 L = light.position - worldPos;
        float distance = length(L);
        if (distance > light.range)
            continue;

        L = normalize(L);
        float attenuation = 1.0 - (distance / light.range);
        attenuation = attenuation * attenuation;

        vec3 radiance = light.color * light.intensity * attenuation;

        vec3 kS;
        vec3 specularBRDF = CookTorrance(normal, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(normal, L), 0.0);
        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL;
    }

    // Spot lights
    for (int i = 0; i < u_SpotLightCount; i++)
    {
        SpotLight light = u_SpotLights[i];

        vec3 L = light.position - worldPos;
        float distance = length(L);
        if (distance > light.range)
            continue;

        L = normalize(L);

        float theta = dot(L, normalize(light.direction));
        float epsilon = light.innerConeAngle - light.outerConeAngle;
        float spotIntensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
        if (spotIntensity <= 0.0)
            continue;

        float attenuation = 1.0 - (distance / light.range);
        attenuation = attenuation * attenuation;
        vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;

        vec3 kS;
        vec3 specularBRDF = CookTorrance(normal, L, V, roughness, metallic, F0, kS);
        vec3 diffuseBRDF = LambertDiffuse(kS, albedo, metallic);

        float NdotL = max(dot(normal, L), 0.0);
        Lo += (diffuseBRDF + specularBRDF) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.0);
    if (u_UseIBL)
    {
        float NoV = max(dot(normal, V), 0.0);
        vec3 F = fresnelSchlickRoughness(NoV, F0, roughness);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
        if (length(irradiance) < 0.001)
            irradiance = vec3(0.03);

        vec3 diffuse = irradiance * albedo;

        vec3 R = reflect(-V, normal);
        float lod = roughness * u_PrefilterMaxLOD;

        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, lod).rgb;
        if (length(prefilteredColor) < 0.001)
            prefilteredColor = vec3(0.03);

        vec2 brdf = texture(u_BRDFLT, vec2(NoV, roughness)).rg;
        float specOcclusion = computeSpecularOcclusion(NoV, combinedAO, roughness);
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * specOcclusion;

        ambient = (kD * diffuse + specular) * combinedAO;
    }
    else
    {
        ambient = vec3(u_AmbientIntensity) * albedo * combinedAO;
    }

    vec3 ssgi = vec3(0.0);
    if (u_EnableSSGI)
        ssgi = texture(u_SSGI, v_TexCoords).rgb;
    ambient += ssgi * combinedAO;

    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
}
