#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in int  a_ObjectID;

uniform mat4 u_ViewProjection;

out vec3 v_Normal;
out vec4 v_Color;
flat out int v_ObjectID;

void main()
{
    v_Normal = a_Normal;
    v_Color = a_Color;
    v_ObjectID = a_ObjectID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
#type fragment
#version 450 core
in vec3 v_Normal;
in vec4 v_Color;
flat in int v_ObjectID;

out vec4 color;

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(v_Normal), lightDir), 0.2);
    color = vec4(v_Color.rgb * diff, v_Color.a);
}
