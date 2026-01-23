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

uniform sampler2D u_GBufferAlbedo;
uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferDepth;
uniform sampler2D u_SSGIHistory;

uniform mat4 u_ViewProjection;
uniform mat4 u_InverseViewProjection;

uniform float u_SSGIRadius;
uniform float u_SSGIBias;
uniform float u_SSGIIntensity;
uniform int u_SSGISampleCount;
uniform int u_FrameIndex;

const int SSGI_MAX_SAMPLES = 64;
const float PI = 3.14159265359;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float halton(int index, int base)
{
    float f = 1.0;
    float r = 0.0;
    int i = index;
    while (i > 0)
    {
        f /= float(base);
        r += f * float(i % base);
        i /= base;
    }
    return r;
}

vec3 cosineSampleHemisphere(vec2 xi)
{
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt(1.0 - xi.y);
    float sinTheta = sqrt(xi.y);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
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
    if (sampleCount > SSGI_MAX_SAMPLES)
        sampleCount = SSGI_MAX_SAMPLES;

    vec2 sequenceOffset = vec2(
        rand(v_TexCoords * 17.13 + float(u_FrameIndex)),
        rand(v_TexCoords * 91.27 + float(u_FrameIndex) * 1.37)
    );

    for (int i = 0; i < SSGI_MAX_SAMPLES; ++i)
    {
        if (i >= sampleCount)
            break;

        int sequenceIndex = i + u_FrameIndex * sampleCount;
        vec2 xi = vec2(halton(sequenceIndex + 1, 2), halton(sequenceIndex + 1, 3));
        float radiusSample = halton(sequenceIndex + 1, 5);
        xi = fract(xi + sequenceOffset);
        radiusSample = fract(radiusSample + sequenceOffset.x);

        vec3 sampleDir = cosineSampleHemisphere(xi);
        vec3 sampleDirWorld = TBN * sampleDir;
        float radiusScale = mix(0.1, 1.0, radiusSample * radiusSample);
        vec3 samplePos = worldPos + sampleDirWorld * radius * radiusScale;

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
        float NdotL = dot(normal, toSampleDir);
        if (NdotL <= 0.0)
            continue;
        float rangeWeight = 1.0 - (dist / radius);
        float weight = rangeWeight;
        if (weight <= 0.0)
            continue;

        vec3 sampleAlbedo = texture(u_GBufferAlbedo, sampleUV).rgb;
        giAccum += sampleAlbedo * weight;
        weightSum += weight;
    }

    vec3 gi = (weightSum > 0.0) ? (giAccum / weightSum) : vec3(0.0);
    gi = gi * albedo * u_SSGIIntensity;

    if (u_FrameIndex <= 0)
    {
        o_Color = vec4(gi, 1.0);
        return;
    }

    vec3 history = texture(u_SSGIHistory, v_TexCoords).rgb;
    float frameWeight = 1.0 / float(u_FrameIndex + 1);
    vec3 giTemporal = mix(history, gi, frameWeight);

    o_Color = vec4(giTemporal, 1.0);
}
