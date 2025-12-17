#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform float u_OutlineWidth = 0.02;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(u_Model)));
    vec3 worldNormal = normalize(normalMatrix * a_Normal);
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    worldPos.xyz += worldNormal * u_OutlineWidth;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

uniform vec4 u_OutlineColor = vec4(0.0, 0.0, 0.0, 1.0);
uniform int u_ObjectID = -1;

void main()
{
    o_Color = u_OutlineColor;
    o_ObjectID = u_ObjectID;
}
