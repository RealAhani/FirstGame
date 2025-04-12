#version 100

precision mediump float;
// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform vec2 iRes;
// uniform vec2  iMouse;
// uniform float iTime;

// uniform sampler2D texture0;
// uniform vec4      colDiffuse;

float invert(float a)
{
    return 1.0 - a;
}
// vec2 invert(vec2 a)
// {
//     return 1.0 - a;
// }
// float CalcAspectRatio()
// {
//     return iRes.x / iRes.y;
// }

// clang-format off
const float Radious = 1.9;
// clang-format on

void main()
{
    // vec2 uv = gl_FragCoord.xy / iRes.xy;
    vec2 uv = fragTexCoord.xy;
    // uv.y    = invert(uv.y);
    uv -= 0.5;
    // uv.x *= CalcAspectRatio();

    float R = length(uv * Radious);
    R *= smoothstep(0.1, 1., R * 2.5 * Radious);

    R = invert(R);

    vec3 scaler  = vec3(10., 20., 30.);
    gl_FragColor = vec4(vec3(R) * scaler * fragColor.rgb, R);
}