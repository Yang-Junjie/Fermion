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
uniform sampler2D u_GBufferDepth;

uniform mat4 u_ViewProjection;
uniform mat4 u_InverseViewProjection;

uniform float u_SSGIRadius;
uniform float u_SSGIBias;
uniform float u_SSGIIntensity;
uniform int u_SSGISampleCount;

const int SSGI_KERNEL_SIZE = 32;
const vec3 SSGI_KERNEL[SSGI_KERNEL_SIZE] = vec3[](
    vec3(0.5381, 0.1856, 0.4319),
    vec3(0.1379, 0.2486, 0.4430),
    vec3(0.3371, 0.5679, 0.0057),
    vec3(0.6999, -0.0451, 0.0019),
    vec3(0.0689, -0.1598, 0.8547),
    vec3(0.0560, 0.0069, 0.1843),
    vec3(-0.0146, 0.1402, 0.0762),
    vec3(0.0100, -0.1924, 0.0344),
    vec3(-0.3577, -0.5301, 0.4358),
    vec3(-0.3169, 0.1063, 0.0158),
    vec3(0.0103, -0.5869, 0.0046),
    vec3(-0.0897, -0.4940, 0.3287),
    vec3(0.7119, -0.0154, 0.0918),
    vec3(-0.0533, 0.0596, 0.5411),
    vec3(0.0353, -0.0631, 0.5460),
    vec3(-0.4776, 0.2847, 0.0271),
    vec3(0.2464, 0.8786, 0.1578),
    vec3(-0.4243, 0.2541, 0.1365),
    vec3(0.0796, 0.4022, 0.8099),
    vec3(0.2998, -0.1527, 0.7442),
    vec3(-0.1221, 0.0524, 0.9891),
    vec3(0.6158, 0.1893, -0.0556),
    vec3(-0.2456, -0.0942, 0.7147),
    vec3(-0.6231, 0.0076, 0.1899),
    vec3(0.0845, -0.8601, 0.2459),
    vec3(-0.1723, -0.2335, 0.6554),
    vec3(0.4027, 0.0931, 0.5984),
    vec3(-0.5582, 0.3812, 0.0473),
    vec3(0.1904, -0.3562, 0.6991),
    vec3(-0.0193, 0.6729, 0.2947),
    vec3(0.7214, -0.1038, 0.2725),
    vec3(-0.3017, -0.6219, 0.1864)
);

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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
    if (length(normal) < 0.1)
    {
        o_Color = vec4(0.0);
        return;
    }

    vec3 worldPos = reconstructWorldPosition(v_TexCoords, depth);

    vec3 randomVec = normalize(vec3(
        rand(v_TexCoords * 12.9898),
        rand(v_TexCoords * 78.233),
        rand(v_TexCoords * 39.425)
    ) * 2.0 - 1.0);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float radius = max(u_SSGIRadius, 0.001);
    float bias = max(u_SSGIBias, 0.0);

    vec3 giAccum = vec3(0.0);
    float weightSum = 0.0;

    int sampleCount = u_SSGISampleCount;
    if (sampleCount < 1)
        sampleCount = 1;
    if (sampleCount > SSGI_KERNEL_SIZE)
        sampleCount = SSGI_KERNEL_SIZE;

    for (int i = 0; i < SSGI_KERNEL_SIZE; ++i)
    {
        if (i >= sampleCount)
            break;

        float scale = float(i) / float(SSGI_KERNEL_SIZE);
        scale = mix(0.1, 1.0, scale * scale);
        vec3 sampleDir = TBN * normalize(SSGI_KERNEL[i]);
        vec3 samplePos = worldPos + sampleDir * radius * scale;

        vec4 offset = u_ViewProjection * vec4(samplePos, 1.0);
        if (offset.w <= 0.0)
            continue;

        vec3 ndc = offset.xyz / offset.w;
        vec2 sampleUV = ndc.xy * 0.5 + 0.5;

        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
            continue;

        float sampleDepth = texture(u_GBufferDepth, sampleUV).r;
        if (sampleDepth >= 1.0)
            continue;

        vec3 sampleWorldPos = reconstructWorldPosition(sampleUV, sampleDepth);
        vec3 toSample = sampleWorldPos - worldPos;
        float dist = length(toSample);
        if (dist <= bias || dist > radius)
            continue;

        vec3 toSampleDir = toSample / max(dist, 0.0001);
        float NdotL = max(dot(normal, toSampleDir), 0.0);
        float rangeWeight = 1.0 - (dist / radius);
        float weight = NdotL * rangeWeight;
        if (weight <= 0.0)
            continue;

        vec3 sampleAlbedo = texture(u_GBufferAlbedo, sampleUV).rgb;
        giAccum += sampleAlbedo * weight;
        weightSum += weight;
    }

    vec3 gi = (weightSum > 0.0) ? (giAccum / weightSum) : vec3(0.0);
    gi = gi * albedo * u_SSGIIntensity;

    o_Color = vec4(gi, 1.0);
}
