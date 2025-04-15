// https://www.shadertoy.com/view/WfXGWs

#version 330
// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
// Input uniform values
uniform vec2  iRes;
uniform float iTime;

out vec4 finalColor;


void main()
{
    vec2  uv         = fragTexCoord.xy - 0.5;
    vec3  outerColor = vec3(0.678, 0.000, 0.000);
    float radius     = .3;

    float wave = 0.05 * sin(uv.y * 10.0 + iTime * 3.0) +
                 0.05 * cos(uv.x * 8.0 + iTime * 7.0);

    radius += wave;

    float dist  = length(uv);
    float c     = smoothstep(radius + .025, radius, dist);
    float alpha = mix(0.0, 1.0, c);
    vec3  color = outerColor;
    if (dist <= radius - .09)
    {
        color = outerColor;
    }
    finalColor = vec4(color, 1.0 * alpha);
}