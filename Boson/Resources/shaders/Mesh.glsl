
#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

out vec3 v_Normal;
out vec4 v_Color;
out vec2 v_TexCoords;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_Color = a_Color;
    v_TexCoords = a_TexCoords;

    gl_Position = u_ViewProjection * worldPos;
}


#type fragment
#version 330 core

in vec3 v_Normal;
in vec4 v_Color;
in vec2 v_TexCoords;

uniform bool u_UseTexture = false;
uniform sampler2D u_Texture;
uniform vec4 u_Kd = vec4(1.0); // 漫反射
uniform vec4 u_Ka = vec4(0.0); // 环境光
uniform bool u_FlipUV = false;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(v_Normal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normal, lightDir), 0.0);

    vec2 uv = v_TexCoords;
    if(u_FlipUV) uv.y = 1.0 - uv.y;

    vec3 color;

    if(u_UseTexture)
    {
        // 贴图材质直接用纹理颜色，不受 Kd 影响
        color = texture(u_Texture, uv).rgb;
    }
    else
    {
        // 无贴图用 Kd + Ka
        color = vec3(u_Kd) + vec3(u_Ka);
    }

    vec3 ambient = 0.3 * color;
    vec3 diffuse = diff * color;

    FragColor = vec4(ambient + diffuse, u_Kd.a);
}

