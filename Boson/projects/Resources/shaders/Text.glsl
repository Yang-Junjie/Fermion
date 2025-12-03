#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_ObjectID;

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
flat out int v_ObjectID;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_ObjectID = a_ObjectID;

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

in vec4 v_Color;
in vec2 v_TexCoord;
flat in int v_ObjectID;

uniform sampler2D u_Atlas;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msd = texture(u_Atlas, v_TexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    // Smooth threshold around 0.5 (MSDF center)
    float alpha = smoothstep(0.5 - 0.1, 0.5 + 0.1, sd);
    if (alpha <= 0.0)
        discard;

    o_Color = vec4(v_Color.rgb, v_Color.a * alpha);
    o_ObjectID = v_ObjectID;
}

