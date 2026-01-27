
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_ObjectID;

// Camera uniform buffer (binding = 0)
layout(std140, binding = 0) uniform CameraData
{
	mat4 u_ViewProjection;
	mat4 u_View;
	mat4 u_Projection;
	vec3 u_CameraPosition;
};

struct VertexOutput {
    vec4 Color;
};

layout(location = 0) out VertexOutput Output;
layout(location = 1) out flat int v_ObjectID;

void main() {
    Output.Color = a_Color;
    v_ObjectID = a_ObjectID;

    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

struct VertexOutput {
    vec4 Color;
};

layout(location = 0) in VertexOutput Input;
layout(location = 1) in flat int v_ObjectID;

void main() {
    o_Color = Input.Color;
    o_ObjectID = v_ObjectID;
}
