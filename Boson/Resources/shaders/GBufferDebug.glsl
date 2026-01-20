#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoords;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoords;

uniform sampler2D u_GBufferAlbedo;
uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferMaterial;
uniform sampler2D u_GBufferEmissive;
uniform sampler2D u_GBufferDepth;
uniform isampler2D u_GBufferObjectID;
uniform sampler2D u_SSGI;
uniform sampler2D u_GTAO;

uniform int u_Mode;
uniform float u_Near;
uniform float u_Far;
uniform float u_DepthPower;

float LinearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

vec3 EncodeObjectID(int id)
{
    if (id < 0)
        return vec3(0.0);

    int r = (id >> 0) & 0xFF;
    int g = (id >> 8) & 0xFF;
    int b = (id >> 16) & 0xFF;
    return vec3(float(r), float(g), float(b)) / 255.0;
}

void main()
{
    vec3 result = vec3(0.0);

    if (u_Mode == 1)
    {
        vec3 albedo = texture(u_GBufferAlbedo, v_TexCoords).rgb;
        result = pow(albedo, vec3(1.0 / 2.2));
    }
    else if (u_Mode == 2)
    {
        vec3 normal = normalize(texture(u_GBufferNormal, v_TexCoords).rgb);
        result = normal * 0.5 + 0.5;
    }
    else if (u_Mode == 3)
    {
        result = texture(u_GBufferMaterial, v_TexCoords).rgb;
    }
    else if (u_Mode == 4)
    {
        float roughness = texture(u_GBufferMaterial, v_TexCoords).r;
        result = vec3(roughness);
    }
    else if (u_Mode == 5)
    {
        float metallic = texture(u_GBufferMaterial, v_TexCoords).g;
        result = vec3(metallic);
    }
    else if (u_Mode == 6)
    {
        float ao = texture(u_GBufferMaterial, v_TexCoords).b;
        result = vec3(ao);
    }
    else if (u_Mode == 7)
    {
        vec3 emissive = texture(u_GBufferEmissive, v_TexCoords).rgb;
        result = pow(emissive, vec3(1.0 / 2.2));
    }
    else if (u_Mode == 8)
    {
        float depth = texture(u_GBufferDepth, v_TexCoords).r;
        float depth01 = LinearizeDepth(depth) / max(u_Far, 0.0001);
        depth01 = 1.0 - clamp(depth01, 0.0, 1.0);
        depth01 = pow(depth01, max(u_DepthPower * 10.0, 0.0001));
        result = vec3(depth01);
    }
    else if (u_Mode == 9)
    {
        int id = texture(u_GBufferObjectID, v_TexCoords).r;
        result = EncodeObjectID(id);
    }
    else if (u_Mode == 10)
    {
        vec3 ssgi = texture(u_SSGI, v_TexCoords).rgb;
        result = pow(clamp(ssgi, 0.0, 1.0), vec3(1.0 / 2.2));
    }
    else if (u_Mode == 11)
    {
        float gtao = texture(u_GTAO, v_TexCoords).r;
        result = vec3(gtao);
    }

    o_Color = vec4(result, 1.0);
}
