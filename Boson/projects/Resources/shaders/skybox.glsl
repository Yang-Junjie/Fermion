#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;

out vec3 v_TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    v_TexCoords = a_Position;

    mat4 view = mat4(mat3(u_View));
    vec4 clipPos = u_Projection * view * vec4(a_Position, 1.0);
    gl_Position = vec4(clipPos.xy, clipPos.w, clipPos.w); // Force depth to far plane to avoid occluding scene geometry
}

#type fragment
#version 330 core
in vec3 v_TexCoords;
out vec4 FragColor;

uniform samplerCube u_Cubemap;

void main() {
    vec3 color = texture(u_Cubemap, v_TexCoords).rgb;
    
    // HDR色调映射 (Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma校正
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, 1.0);
}
