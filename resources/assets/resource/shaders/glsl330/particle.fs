#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform vec2 iRes;

out vec4 finalColor;

float invert(float a)
{
    return 1.0 - a;
}

// clang-format off
const float Radious = 2.0;
// clang-format on

void main()
{
    vec2 uv = fragTexCoord.xy;
    uv -= 0.5;

    float R           = length(uv * Radious);
    float innerRadios = 0.11;
    R *= smoothstep(0.1, innerRadios, R * Radious);

    R = invert(R);

    vec3 scaler = vec3(10., 20., 30.);
    finalColor  = vec4(vec3(R) * scaler * fragColor.rgb, R);
}