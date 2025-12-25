#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform int u_ObjectID;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec4 v_Color;
out vec2 v_TexCoords;
flat out int v_ObjectID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;

    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_Color = a_Color;
    v_TexCoords = a_TexCoords;
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
flat in int v_ObjectID;

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

uniform bool u_UseTexture;
uniform sampler2D u_Texture;

uniform vec4 u_Kd; // diffuse
uniform vec4 u_Ka; // ambient
uniform bool u_FlipUV;

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
