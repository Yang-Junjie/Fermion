#type vertex
#version 450 core

// [已弃用] 此着色器不再用于描边渲染。
// 选中物体改为通过Renderer2D绘制AABB包围盒线框。
// 保留此文件以避免着色器加载错误。

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

void main()
{
    discard;
}
