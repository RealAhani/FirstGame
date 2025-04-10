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

void main()
{
    vec2 uv = fragTexCoord.xy;

    uv -= .5;

    float rx = length(uv) / 2.;

    float sx = smoothstep(.2, .3, rx);

    float nx = (1. - sx) * (sin(iTime));

    finalColor = vec4(vec3(nx * 1.5) * fragColor.rgb, 1.);
}
