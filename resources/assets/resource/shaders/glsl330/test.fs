#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform vec2  iRes;
uniform vec2  iMouse;
uniform float iTime;

uniform sampler2D texture0;
uniform vec4      colDiffuse;

// Output fragment color
out vec4 finalColor;
float    invert(float a)
{
    return 1.0 - a;
}
vec2 invert(vec2 a)
{
    return 1.0 - a;
}
float CalcAspectRatio()
{
    return iRes.x / iRes.y;
}

// clang-format off
const float Radious = 1.9;
// clang-format on

void main()
{
    vec2 uv = fragTexCoord;
    uv.y    = invert(uv.y);
    uv -= 0.5;
    // uvx *= CalcAspectRatio();

    float R = length(uv * Radious);
    R *= smoothstep(0.1, 1., R * 2.5 * Radious);

    R = invert(R);

    // vec3 scaler = vec3(1., 2., 3.);
    finalColor = vec4(vec3(R) * fragColor.rgb, R);
}