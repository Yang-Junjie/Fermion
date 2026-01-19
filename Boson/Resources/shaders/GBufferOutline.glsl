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

uniform sampler2D u_GBufferNormal;
uniform sampler2D u_GBufferDepth;
uniform isampler2D u_GBufferObjectID;

uniform vec4 u_OutlineColor;
uniform int u_OutlineIDCount;
uniform int u_OutlineIDs[32];

uniform float u_Near;
uniform float u_Far;
uniform float u_DepthThreshold;
uniform float u_NormalThreshold;
uniform float u_Thickness;

float LinearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

bool IsOutlineID(int id)
{
    for (int i = 0; i < u_OutlineIDCount; ++i)
    {
        if (u_OutlineIDs[i] == id)
            return true;
    }
    return false;
}

void main()
{
    if (u_OutlineIDCount <= 0)
        discard;

    int centerID = texture(u_GBufferObjectID, v_TexCoords).r;
    if (!IsOutlineID(centerID))
        discard;

    float depth = texture(u_GBufferDepth, v_TexCoords).r;
    if (depth >= 1.0)
        discard;

    vec2 texelSize = 1.0 / vec2(textureSize(u_GBufferDepth, 0));
    vec2 offset = texelSize * max(u_Thickness, 1.0);

    vec2 uvL = v_TexCoords + vec2(-offset.x, 0.0);
    vec2 uvR = v_TexCoords + vec2(offset.x, 0.0);
    vec2 uvU = v_TexCoords + vec2(0.0, offset.y);
    vec2 uvD = v_TexCoords + vec2(0.0, -offset.y);

    int idL = texture(u_GBufferObjectID, uvL).r;
    int idR = texture(u_GBufferObjectID, uvR).r;
    int idU = texture(u_GBufferObjectID, uvU).r;
    int idD = texture(u_GBufferObjectID, uvD).r;

    float edgeId = (idL != centerID || idR != centerID || idU != centerID || idD != centerID) ? 1.0 : 0.0;

    float depthCenter = LinearizeDepth(depth) / max(u_Far, 0.0001);
    float depthDiff = 0.0;

    float depthL = LinearizeDepth(texture(u_GBufferDepth, uvL).r) / max(u_Far, 0.0001);
    float depthR = LinearizeDepth(texture(u_GBufferDepth, uvR).r) / max(u_Far, 0.0001);
    float depthU = LinearizeDepth(texture(u_GBufferDepth, uvU).r) / max(u_Far, 0.0001);
    float depthD = LinearizeDepth(texture(u_GBufferDepth, uvD).r) / max(u_Far, 0.0001);

    depthDiff = max(depthDiff, abs(depthCenter - depthL));
    depthDiff = max(depthDiff, abs(depthCenter - depthR));
    depthDiff = max(depthDiff, abs(depthCenter - depthU));
    depthDiff = max(depthDiff, abs(depthCenter - depthD));

    float edgeDepth = step(u_DepthThreshold, depthDiff);

    vec3 nCenter = normalize(texture(u_GBufferNormal, v_TexCoords).rgb);
    vec3 nL = normalize(texture(u_GBufferNormal, uvL).rgb);
    vec3 nR = normalize(texture(u_GBufferNormal, uvR).rgb);
    vec3 nU = normalize(texture(u_GBufferNormal, uvU).rgb);
    vec3 nD = normalize(texture(u_GBufferNormal, uvD).rgb);

    float normalDiff = 0.0;
    normalDiff = max(normalDiff, 1.0 - dot(nCenter, nL));
    normalDiff = max(normalDiff, 1.0 - dot(nCenter, nR));
    normalDiff = max(normalDiff, 1.0 - dot(nCenter, nU));
    normalDiff = max(normalDiff, 1.0 - dot(nCenter, nD));

    float edgeNormal = step(u_NormalThreshold, normalDiff);

    float edge = max(edgeId, max(edgeDepth, edgeNormal));
    float alpha = u_OutlineColor.a * edge;
    if (alpha <= 0.0)
        discard;

    o_Color = vec4(u_OutlineColor.rgb, alpha);
}
