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

uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferDepth;

uniform mat4 u_Projection;
uniform mat4 u_InverseProjection;
uniform mat4 u_View;

uniform float u_GTAORadius;
uniform float u_GTAOBias;
uniform float u_GTAOPower;
uniform float u_GTAOIntensity;
uniform int u_GTAOSliceCount;
uniform int u_GTAOStepCount;

const int GTAO_MAX_SLICES = 12;
const int GTAO_MAX_STEPS = 16;
const float PI = 3.14159265359;
const float PI_HALF = 1.57079632679;
const float GTAO_RADIUS_MULTIPLIER = 1.457;
const float GTAO_FALLOFF_RANGE = 0.615;
const float GTAO_SAMPLE_DISTRIBUTION_POWER = 2.0;
const float GTAO_THIN_OCCLUDER_COMPENSATION = 0.0;
const float GTAO_PIXEL_TOO_CLOSE_THRESHOLD = 1.3;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 reconstructViewPosition(vec2 uv, float depth)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = u_InverseProjection * clip;
    return view.xyz / max(view.w, 0.0001);
}

void main()
{
    float depth = texture(u_GBufferDepth, v_TexCoords).r;
    if (depth >= 1.0)
    {
        o_Color = vec4(1.0);
        return;
    }

    vec3 normalWorld = normalize(texture(u_GBufferNormal, v_TexCoords).rgb);
    if (length(normalWorld) < 0.1)
    {
        o_Color = vec4(1.0);
        return;
    }

    vec3 viewPos = reconstructViewPosition(v_TexCoords, depth);
    vec3 normalView = normalize(mat3(u_View) * normalWorld);
    vec3 viewDir = normalize(-viewPos);

    float radius = max(u_GTAORadius, 0.001) * GTAO_RADIUS_MULTIPLIER;
    float bias = max(u_GTAOBias, 0.0);

    vec2 projScale = vec2(u_Projection[0][0], u_Projection[1][1]);
    vec2 radiusUV = 0.5 * radius * projScale / max(-viewPos.z, 0.001);

    vec2 texSize = vec2(textureSize(u_GBufferDepth, 0));
    float screenRadius = max(radiusUV.x * texSize.x, 0.001);
    if (screenRadius < GTAO_PIXEL_TOO_CLOSE_THRESHOLD)
    {
        o_Color = vec4(1.0);
        return;
    }

    float minS = clamp(GTAO_PIXEL_TOO_CLOSE_THRESHOLD / screenRadius, 0.0, 1.0);

    float falloffRange = radius * GTAO_FALLOFF_RANGE;
    float falloffFrom = radius * (1.0 - GTAO_FALLOFF_RANGE);
    float falloffMul = -1.0 / max(falloffRange, 0.0001);
    float falloffAdd = falloffFrom / max(falloffRange, 0.0001) + 1.0;

    int sliceCount = clamp(u_GTAOSliceCount, 1, GTAO_MAX_SLICES);
    int stepCount = clamp(u_GTAOStepCount, 1, GTAO_MAX_STEPS);

    vec2 noiseUv = v_TexCoords * texSize;
    float noiseSlice = rand(noiseUv);
    float noiseSample = rand(noiseUv.yx + 13.57);

    float visibility = 0.0;

    for (int i = 0; i < GTAO_MAX_SLICES; ++i)
    {
        if (i >= sliceCount)
            break;

        float sliceK = (float(i) + noiseSlice) / float(sliceCount);
        float phi = sliceK * PI;
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        vec2 dir = vec2(cosPhi, sinPhi);

        vec3 directionVec = vec3(cosPhi, sinPhi, 0.0);
        vec3 orthoDirectionVec = directionVec - dot(directionVec, viewDir) * viewDir;
        vec3 axisVec = normalize(cross(orthoDirectionVec, viewDir));
        vec3 projectedNormal = normalView - axisVec * dot(normalView, axisVec);

        float projectedNormalLen = length(projectedNormal);
        if (projectedNormalLen < 1e-4)
            continue;

        float cosNorm = clamp(dot(projectedNormal, viewDir) / projectedNormalLen, -1.0, 1.0);
        if (cosNorm <= 0.0)
            continue;

        float signNorm = sign(dot(orthoDirectionVec, projectedNormal));
        float n = signNorm * acos(cosNorm);

        float lowHorizonCos0 = cos(n + PI_HALF);
        float lowHorizonCos1 = cos(n - PI_HALF);
        float horizonCos0 = lowHorizonCos0;
        float horizonCos1 = lowHorizonCos1;

        for (int s = 0; s < GTAO_MAX_STEPS; ++s)
        {
            if (s >= stepCount)
                break;

            float stepBaseNoise = (float(i) + float(s) * float(stepCount)) * 0.61803398875;
            float stepNoise = fract(noiseSample + stepBaseNoise);
            float t = (float(s) + stepNoise) / float(stepCount);
            t = pow(t, GTAO_SAMPLE_DISTRIBUTION_POWER);
            t = minS + t * (1.0 - minS);

            vec2 offset = dir * radiusUV * t;

            vec2 uvPos = v_TexCoords + offset;
            if (uvPos.x >= 0.0 && uvPos.x <= 1.0 && uvPos.y >= 0.0 && uvPos.y <= 1.0)
            {
                float sampleDepth = texture(u_GBufferDepth, uvPos).r;
                if (sampleDepth < 1.0)
                {
                    vec3 samplePos = reconstructViewPosition(uvPos, sampleDepth);
                    vec3 delta = samplePos - viewPos;
                    float dist = length(delta);
                    if (dist > bias && dist <= radius)
                    {
                        float falloffBase = length(vec3(delta.x, delta.y, delta.z * (1.0 + GTAO_THIN_OCCLUDER_COMPENSATION)));
                        float weight = clamp(falloffBase * falloffMul + falloffAdd, 0.0, 1.0);
                        float depthDiff = abs(viewPos.z - samplePos.z);
                        float rangeCheck = smoothstep(0.0, 1.0, radius / max(depthDiff, 0.0001));
                        weight *= rangeCheck;

                        vec3 horizonVec = delta / max(dist, 0.0001);
                        float shc = dot(horizonVec, viewDir);
                        shc = mix(lowHorizonCos0, shc, weight);
                        horizonCos0 = max(horizonCos0, shc);
                    }
                }
            }

            vec2 uvNeg = v_TexCoords - offset;
            if (uvNeg.x >= 0.0 && uvNeg.x <= 1.0 && uvNeg.y >= 0.0 && uvNeg.y <= 1.0)
            {
                float sampleDepth = texture(u_GBufferDepth, uvNeg).r;
                if (sampleDepth < 1.0)
                {
                    vec3 samplePos = reconstructViewPosition(uvNeg, sampleDepth);
                    vec3 delta = samplePos - viewPos;
                    float dist = length(delta);
                    if (dist > bias && dist <= radius)
                    {
                        float falloffBase = length(vec3(delta.x, delta.y, delta.z * (1.0 + GTAO_THIN_OCCLUDER_COMPENSATION)));
                        float weight = clamp(falloffBase * falloffMul + falloffAdd, 0.0, 1.0);
                        float depthDiff = abs(viewPos.z - samplePos.z);
                        float rangeCheck = smoothstep(0.0, 1.0, radius / max(depthDiff, 0.0001));
                        weight *= rangeCheck;

                        vec3 horizonVec = delta / max(dist, 0.0001);
                        float shc = dot(horizonVec, viewDir);
                        shc = mix(lowHorizonCos1, shc, weight);
                        horizonCos1 = max(horizonCos1, shc);
                    }
                }
            }
        }

        float h0 = -acos(clamp(horizonCos1, -1.0, 1.0));
        float h1 = acos(clamp(horizonCos0, -1.0, 1.0));
        float iarc0 = (cosNorm + 2.0 * h0 * sin(n) - cos(2.0 * h0 - n)) * 0.25;
        float iarc1 = (cosNorm + 2.0 * h1 * sin(n) - cos(2.0 * h1 - n)) * 0.25;

        visibility += projectedNormalLen * (iarc0 + iarc1);
    }

    visibility = max(visibility / float(sliceCount), 0.0);
    visibility = max(0.03, visibility);
    visibility = pow(visibility, max(u_GTAOPower, 0.01));

    float ao = clamp(1.0 - (1.0 - visibility) * u_GTAOIntensity, 0.0, 1.0);
    o_Color = vec4(vec3(ao), 1.0);
}
