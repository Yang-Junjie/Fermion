#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform int u_ObjectID;

out vec3 v_Normal;
out vec2 v_TexCoords;
flat out int v_ObjectID;

void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoords = a_TexCoords;
    v_ObjectID = u_ObjectID;

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Material;
layout(location = 3) out vec4 o_Emissive;
layout(location = 4) out int o_ObjectID;

in vec3 v_Normal;
in vec2 v_TexCoords;
flat in int v_ObjectID;

uniform bool u_UseTexture;
uniform sampler2D u_Texture;
uniform vec4 u_Kd;
uniform bool u_FlipUV;

void main()
{
    vec2 uv = v_TexCoords;
    if (u_FlipUV)
        uv.y = 1.0 - uv.y;

    vec4 baseColor = u_UseTexture ? texture(u_Texture, uv) : u_Kd;

    o_Albedo = vec4(baseColor.rgb, baseColor.a);
    o_Normal = vec4(normalize(v_Normal), 1.0);
    o_Material = vec4(1.0, 0.0, 1.0, 1.0);
    o_Emissive = vec4(0.0, 0.0, 0.0, 1.0);
    o_ObjectID = v_ObjectID;
}
