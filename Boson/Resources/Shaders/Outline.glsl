#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

uniform float u_OutlineWidth;
uniform float u_Epsilon;

void main() {

    vec4 viewPos = u_View * u_Model * vec4(a_Position, 1.0);

    mat3 normalMatrix = mat3(transpose(inverse(u_View * u_Model)));
    vec3 viewNormal = normalize(normalMatrix * a_Normal);

    float ndotv = abs(dot(viewNormal, vec3(0.0, 0.0, -1.0)));
    float outlineFactor = 1.0 - smoothstep(0.0, u_Epsilon, ndotv);

    vec3 expandNormal = normalize(vec3(viewNormal.xy, 0.0));
    float depth = -viewPos.z;
    float depthScale = clamp(depth * 0.02, 0.5, 2.0);

    viewPos.xyz += expandNormal * u_OutlineWidth * depthScale * outlineFactor;

    gl_Position = u_Projection * viewPos;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

uniform vec4 u_OutlineColor = vec4(0.0, 0.0, 0.0, 1.0);
uniform int u_ObjectID = -1;

void main() {
    o_Color = u_OutlineColor;
    o_ObjectID = u_ObjectID;
}
