// Infinite Grid Shader
// Based on "The Best Darn Grid Shader (Yet)" by Ben Golus

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_CameraPosition;
};

out vec3 v_NearPoint;
out vec3 v_FarPoint;

vec3 unprojectPoint(float x, float y, float z) {
    mat4 viewInv = inverse(u_View);
    mat4 projInv = inverse(u_Projection);
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    v_NearPoint = unprojectPoint(a_Position.x, a_Position.y, 0.0);
    v_FarPoint = unprojectPoint(a_Position.x, a_Position.y, 1.0);
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_ObjectID;

in vec3 v_NearPoint;
in vec3 v_FarPoint;

layout(std140, binding = 0) uniform CameraData
{
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_CameraPosition;
};

// Grid plane: 0 = XZ (Y up), 1 = XY (Z forward), 2 = YZ (X right)
uniform int u_GridPlane = 0;
uniform float u_GridScale = 1.0;
uniform vec4 u_GridColorThin = vec4(0.5, 0.5, 0.5, 0.4);
uniform vec4 u_GridColorThick = vec4(0.5, 0.5, 0.5, 0.6);
uniform vec4 u_AxisColorX = vec4(0.9, 0.2, 0.2, 1.0);
uniform vec4 u_AxisColorZ = vec4(0.2, 0.2, 0.9, 1.0);
uniform float u_FadeDistance = 500.0;

float computeDepth(vec3 pos) {
    vec4 clipSpacePos = u_ViewProjection * vec4(pos, 1.0);
    return clipSpacePos.z / clipSpacePos.w;
}

// Get the plane normal based on grid plane type
vec3 getPlaneNormal() {
    if (u_GridPlane == 1) return vec3(0.0, 0.0, 1.0);  // XY plane, Z normal
    if (u_GridPlane == 2) return vec3(1.0, 0.0, 0.0);  // YZ plane, X normal
    return vec3(0.0, 1.0, 0.0);  // XZ plane, Y normal (default)
}

// Get 2D coordinates on the plane
vec2 getPlaneCoords(vec3 pos) {
    if (u_GridPlane == 1) return pos.xy;  // XY plane
    if (u_GridPlane == 2) return pos.yz;  // YZ plane
    return pos.xz;  // XZ plane (default)
}

// Get the component perpendicular to the plane (for axis drawing)
vec2 getAxisCoords(vec3 pos) {
    // Returns the two coordinates used for drawing axis lines
    // First component -> first axis color, Second component -> second axis color
    if (u_GridPlane == 1) return vec2(pos.x, pos.y);  // XY: X-axis and Y-axis
    if (u_GridPlane == 2) return vec2(pos.y, pos.z);  // YZ: Y-axis and Z-axis
    return vec2(pos.x, pos.z);  // XZ: X-axis and Z-axis
}

// Calculate t for ray-plane intersection
float rayPlaneIntersection(vec3 nearPoint, vec3 farPoint) {
    vec3 rayDir = farPoint - nearPoint;

    if (u_GridPlane == 1) {
        // XY plane (z = 0)
        if (abs(rayDir.z) < 0.0001) return -1.0;
        return -nearPoint.z / rayDir.z;
    }
    if (u_GridPlane == 2) {
        // YZ plane (x = 0)
        if (abs(rayDir.x) < 0.0001) return -1.0;
        return -nearPoint.x / rayDir.x;
    }
    // XZ plane (y = 0) - default
    if (abs(rayDir.y) < 0.0001) return -1.0;
    return -nearPoint.y / rayDir.y;
}

// Get view angle component for normal fade
float getViewAngleComponent(vec3 viewDir) {
    if (u_GridPlane == 1) return abs(viewDir.z);  // XY plane
    if (u_GridPlane == 2) return abs(viewDir.x);  // YZ plane
    return abs(viewDir.y);  // XZ plane
}

// Pristine grid - single pixel line with proper AA
float pristineGridLine(vec2 uv) {
    vec2 dudv = fwidth(uv);
    vec2 uvMod = fract(uv);
    vec2 uvDist = min(uvMod, 1.0 - uvMod);
    vec2 distInPixels = uvDist / dudv;
    vec2 lineAlpha = 1.0 - smoothstep(0.0, 1.0, distInPixels);
    float alpha = max(lineAlpha.x, lineAlpha.y);
    float density = max(dudv.x, dudv.y);
    float densityFade = 1.0 - smoothstep(0.5, 1.0, density);
    return alpha * densityFade;
}

// Axis line - single pixel wide
float axisLineAA(float coord, float dudv) {
    float distInPixels = abs(coord) / dudv;
    return 1.0 - smoothstep(0.0, 1.5, distInPixels);
}

void main() {
    float t = rayPlaneIntersection(v_NearPoint, v_FarPoint);

    if (t < 0.0) {
        discard;
    }

    vec3 fragPos3D = v_NearPoint + t * (v_FarPoint - v_NearPoint);
    float depth = computeDepth(fragPos3D);

    if (depth > 1.0 || depth < -1.0) {
        discard;
    }

    vec2 worldPos = getPlaneCoords(fragPos3D);

    // === Fading ===

    // Radial fade
    float dist = length(fragPos3D - u_CameraPosition);
    float radialFade = 1.0 - smoothstep(u_FadeDistance * 0.3, u_FadeDistance, dist);

    // Normal fade (view angle)
    vec3 viewDir = normalize(fragPos3D - u_CameraPosition);
    float viewAngle = getViewAngleComponent(viewDir);
    float normalFade = smoothstep(0.0, 0.15, viewAngle);

    float fadeFactor = radialFade * normalFade;

    if (fadeFactor < 0.001) {
        discard;
    }

    // === Grid calculation ===

    vec2 gridCoord1 = worldPos / u_GridScale;
    vec2 gridCoord10 = worldPos / (u_GridScale * 10.0);

    float grid1 = pristineGridLine(gridCoord1);
    float grid10 = pristineGridLine(gridCoord10);

    // LOD blend
    vec2 deriv1 = fwidth(gridCoord1);
    float lodFactor = smoothstep(0.3, 0.6, max(deriv1.x, deriv1.y));

    // Combine grids
    float gridIntensity = mix(max(grid1, grid10 * 0.7), grid10, lodFactor);

    // Grid color
    vec3 gridColor = mix(u_GridColorThin.rgb, u_GridColorThick.rgb, lodFactor);
    float baseAlpha = mix(u_GridColorThin.a, u_GridColorThick.a, lodFactor);
    float gridAlpha = baseAlpha * gridIntensity * fadeFactor;

    // === Axis lines ===

    vec2 axisCoords = getAxisCoords(fragPos3D);
    vec2 worldDeriv = fwidth(worldPos);

    // First axis (uses AxisColorX - typically red)
    float axis1Alpha = axisLineAA(axisCoords.y, worldDeriv.y) * fadeFactor;
    // Second axis (uses AxisColorZ - typically blue)
    float axis2Alpha = axisLineAA(axisCoords.x, worldDeriv.x) * fadeFactor;

    // === Final composition ===

    vec3 finalColor = gridColor;
    float finalAlpha = gridAlpha;

    // Blend axis colors
    if (axis2Alpha > 0.001) {
        float blend = axis2Alpha * u_AxisColorZ.a;
        finalColor = mix(finalColor, u_AxisColorZ.rgb, blend);
        finalAlpha = max(finalAlpha, blend);
    }

    if (axis1Alpha > 0.001) {
        float blend = axis1Alpha * u_AxisColorX.a;
        finalColor = mix(finalColor, u_AxisColorX.rgb, blend);
        finalAlpha = max(finalAlpha, blend);
    }

    if (finalAlpha < 0.001) {
        discard;
    }

    gl_FragDepth = depth * 0.5 + 0.5;
    o_Color = vec4(finalColor, finalAlpha);
    o_ObjectID = -1;
}
