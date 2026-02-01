#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Projection;
uniform mat4 u_View;

out vec3 v_LocalPos;

void main()
{
    v_LocalPos = a_Position;

    // Remove translation from view matrix
    mat4 rotView = mat4(mat3(u_View));
    vec4 clipPos = u_Projection * rotView * vec4(a_Position, 1.0);

    gl_Position = clipPos.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 FragColor;

in vec3 v_LocalPos;

// Sky parameters
uniform vec3 u_SunDirection;       // Normalized sun direction
uniform float u_SunIntensity;       // HDR sun brightness
uniform float u_SunAngularRadius;   // Sun angular radius in radians
uniform vec3 u_SkyColorZenith;      // Zenith color (linear)
uniform vec3 u_SkyColorHorizon;     // Horizon color (linear)
uniform vec3 u_GroundColor;         // Ground color (linear)
uniform float u_SkyExposure;        // Sky exposure multiplier

const float PI = 3.14159265359;

// Rayleigh scattering approximation
// Simulates how shorter wavelengths (blue) scatter more at zenith
vec3 computeRayleighScattering(vec3 direction, vec3 sunDir)
{
    // Height gradient: zenith (y=1) to horizon (y=0)
    float height = max(direction.y, 0.0);

    // Exponential falloff for more natural sky gradient
    float zenithFactor = pow(height, 0.5);

    // Blend between horizon and zenith colors
    vec3 skyColor = mix(u_SkyColorHorizon, u_SkyColorZenith, zenithFactor);

    // Add slight brightening near horizon (atmospheric haze)
    float horizonGlow = exp(-height * 4.0) * 0.3;
    skyColor += u_SkyColorHorizon * horizonGlow;

    return skyColor;
}

// Mie scattering - creates glow around the sun
vec3 computeMieScattering(vec3 direction, vec3 sunDir)
{
    float cosTheta = dot(direction, sunDir);

    // Henyey-Greenstein phase function approximation
    // g controls asymmetry (0.76 gives forward scattering)
    float g = 0.76;
    float g2 = g * g;

    float phase = (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));

    // Scale and color the Mie scattering
    // Warm color for sunset-like glow
    vec3 mieColor = vec3(1.0, 0.9, 0.7);

    return mieColor * phase * 0.15;
}

// Sun disc with soft edge
vec3 computeSunDisc(vec3 direction, vec3 sunDir)
{
    float cosTheta = dot(direction, sunDir);
    float sunCosAngle = cos(u_SunAngularRadius);

    // Soft edge using smoothstep
    float edgeSoftness = u_SunAngularRadius * 0.3;
    float sunDisc = smoothstep(
        cos(u_SunAngularRadius + edgeSoftness),
        sunCosAngle,
        cosTheta
    );

    // HDR sun color (slightly warm)
    vec3 sunColor = vec3(1.0, 0.95, 0.9) * u_SunIntensity;

    return sunColor * sunDisc;
}

void main()
{
    vec3 direction = normalize(v_LocalPos);
    vec3 sunDir = normalize(u_SunDirection);

    vec3 color = vec3(0.0);

    // Check if we're looking at sky or ground
    if (direction.y > -0.02)
    {
        // Sky hemisphere

        // Base sky color with Rayleigh scattering
        vec3 skyColor = computeRayleighScattering(direction, sunDir);

        // Add Mie scattering (sun glow)
        vec3 mie = computeMieScattering(direction, sunDir);
        skyColor += mie;

        // Add sun disc (only if sun is above horizon)
        if (sunDir.y > 0.0)
        {
            vec3 sunDisc = computeSunDisc(direction, sunDir);
            skyColor += sunDisc;
        }

        // Apply exposure
        color = skyColor * u_SkyExposure;

        // Soft transition to ground at horizon
        float horizonBlend = smoothstep(-0.02, 0.05, direction.y);
        color = mix(u_GroundColor * u_SkyExposure * 0.5, color, horizonBlend);
    }
    else
    {
        // Ground hemisphere
        // Darken towards nadir for depth
        float groundFade = smoothstep(-1.0, -0.1, direction.y);
        color = u_GroundColor * u_SkyExposure * 0.5 * groundFade;
    }

    // Output linear HDR (no tonemapping - IBL pipeline will process this)
    FragColor = vec4(color, 1.0);
}
