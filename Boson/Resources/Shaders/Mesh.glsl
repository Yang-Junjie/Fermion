#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;

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
	float _lightPadding1;
	float u_AmbientIntensity;
	int u_NumPointLights;
	int u_NumSpotLights;
};

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_Color;
out vec2 v_TexCoords;
out vec4 v_FragPosLightSpace;
flat out int v_ObjectID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    // Use precomputed normal matrix from UBO
    v_Normal = mat3(u_NormalMatrix) * a_Normal;
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

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16

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

uniform bool u_UseTexture;
uniform sampler2D u_Texture;

uniform vec4 u_Kd; // diffuse
uniform vec4 u_Ka; // ambient
uniform bool u_FlipUV;

// Shadow mapping
uniform sampler2D u_ShadowMap;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // If outside shadow map bounds, assume no shadow
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    // Calculate bias based on surface angle
    float bias = max(u_ShadowBias * (1.0 - dot(normal, lightDir)), u_ShadowBias * 0.1);
    
    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    int pcfRange = int(u_ShadowSoftness);
    int sampleCount = 0;
    
    for(int x = -pcfRange; x <= pcfRange; ++x)
    {
        for(int y = -pcfRange; y <= pcfRange; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            sampleCount++;
        }
    }
    shadow /= float(sampleCount);
    
    return shadow;
}

void main()
{
    vec3 normal = normalize(v_Normal);

    vec2 uv = v_TexCoords;
    if(u_FlipUV)
        uv.y = 1.0 - uv.y;

    vec3 baseColor;
    if(u_UseTexture)
        baseColor = texture(u_Texture, uv).rgb;
    else
        baseColor = u_Kd.rgb;

    // Ambient
    vec3 result = u_Ka.rgb * baseColor;

    // Directional light with shadow
    vec3 dirLightDir = normalize(-u_DirLightDirection); // 方向光方向指向片元
    float NdotL = max(dot(normal, dirLightDir), 0.0);
    float shadow = 0.0;
    if (u_EnableShadows != 0) {
        shadow = calculateShadow(v_FragPosLightSpace, normal, dirLightDir);
    }
    result += u_DirLightColor * u_DirLightIntensity * NdotL * (1.0 - shadow) * baseColor;

    // Point lights
    for(int i = 0; i < u_PointLightCount; i++)
    {
        PointLight light = u_PointLights[i];

        vec3 L = light.position - v_WorldPos;
        float distance = length(L);
        if(distance > light.range)
            continue;

        vec3 lightDir = normalize(L);
        float NdotL = max(dot(normal, lightDir), 0.0);

        float attenuation = 1.0 - distance / light.range;

        result += light.color *
                  light.intensity *
                  NdotL *
                  attenuation *
                  baseColor;
    }

    // Spot lights
    for(int i = 0; i < u_SpotLightCount; i++)
    {
        SpotLight light = u_SpotLights[i];

        vec3 L = light.position - v_WorldPos;
        float distance = length(L);
        if(distance > light.range)
            continue;

        vec3 lightDir = normalize(L);

        float theta = dot(lightDir, normalize(light.direction));
        float innerCos = light.innerConeAngle;
        float outerCos = light.outerConeAngle;

        if(theta < outerCos)
            continue;

        float spot = clamp((theta - outerCos) / (innerCos - outerCos), 0.0, 1.0);

        float NdotL = max(dot(normal, lightDir), 0.0);
        float attenuation = 1.0 - distance / light.range;

        result += light.color *
                  light.intensity *
                  NdotL *
                  attenuation *
                  spot *
                  baseColor;
    }

    o_Color = vec4(result, u_Kd.a);
    o_ObjectID = v_ObjectID;
}
