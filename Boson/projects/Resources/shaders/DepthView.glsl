#type vertex
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0, 1);
}

#type fragment
#version 450 core
out vec4 FragColor;
in vec2 vUV;

uniform sampler2D u_Depth;
uniform float u_Near;
uniform float u_Far;
uniform int u_IsPerspective;
uniform float u_Power;

float LinearizeDepth(float d) {
    float z = d * 2.0 - 1.0;
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
}

void main() {
    float depth = texture(u_Depth, vUV).r;
    float depth01 = (u_IsPerspective != 0) ? (LinearizeDepth(depth) / u_Far) : depth;
    depth01 =1.0 - clamp(depth01, 0.0, 1.0);
    depth01 = pow(depth01, max(u_Power*10.0, 0.0001));
    FragColor = vec4(vec3(depth01), 1.0);
}
