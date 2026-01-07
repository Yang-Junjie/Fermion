// ============================================================================
// 辐照度贴图生成 - 用于漫反射IBL
// ============================================================================
#type vertex
#version 330 core
layout(location = 0) in vec3 a_Position;

out vec3 v_WorldPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main() {
    v_WorldPos = a_Position;
    gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;
in vec3 v_WorldPos;

uniform samplerCube u_EnvironmentMap;

const float PI = 3.14159265359;

void main() {
    // 将片段位置向量作为法线方向
    vec3 N = normalize(v_WorldPos);
    
    vec3 irradiance = vec3(0.0);
    
    // 构建切线空间（处理退化情况）
    vec3 up = vec3(0.0, 1.0, 0.0);
    // 当N接近(0,±1,0)时，使用不同的up向量
    if (abs(N.y) > 0.999) {
        up = vec3(0.0, 0.0, 1.0);
    }
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    
    // 在半球上采样
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // 球面坐标转换为笛卡尔坐标(切线空间)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // 切线空间转世界空间
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            
            // 从environment cubemap采样（保持原始方向）
            irradiance += texture(u_EnvironmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}