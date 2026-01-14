#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;

out vec3 v_LocalPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main() {
    v_LocalPos = a_Position;
    // 只使用view的旋转部分，移除平移（与skybox一致）
    mat4 rotView = mat4(mat3(u_View));
    vec4 clipPos = u_Projection * rotView * vec4(a_Position, 1.0);
    gl_Position = clipPos.xyww; // 防止从立方体内部渲染时被近平面裁剪
}

#type fragment
#version 330 core
out vec4 FragColor;

in vec3 v_LocalPos;

uniform sampler2D u_EquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(v_LocalPos));
    vec3 color = texture(u_EquirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
