// https://www.shadertoy.com/view/3cXSWn
#version 100

precision highp float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform vec2  iRes;
uniform float iTime;
uniform vec3  iColor;


float RandFloat(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec2 RandVec2(vec2 p)
{
    return fract(sin(p * mat2(127.1, 311.7, 269.5, 183.3)) * 43758.5453123);
}

float LineSegment(vec2 p, vec2 a, vec2 b)
{
    vec2  pa = p - a;
    vec2  ba = b - a;
    float t  = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * t);
}

float Line(vec2 p, vec2 a, vec2 b)
{
    float d = LineSegment(p, a, b);
    float w = 10.0 / iRes.y;
    float m = smoothstep(w, -w, d - 0.02);
    m *= smoothstep(2.0, 1.0, length(a - b));
    return m;
}

vec2 GetPointPosition(vec2 id, vec2 offset)
{
    vec2 v = RandVec2(id + offset);
    return offset + sin(v * 6.2831 + iTime) * 0.4;
}

void main()
{
    vec3 resultColor = vec3(0.0);
    vec2 uv          = gl_FragCoord.xy / iRes.xy;
    uv               = 30.0 * (gl_FragCoord.xy - 0.5 * iRes.xy) / iRes.y;

    vec2 id = floor(uv);
    vec2 gv = fract(uv) - 0.5;

    float m = 0.0;
    vec2  p[9];

    int i = 0;
    for (float y = -1.0; y <= 1.0; y++)
    {
        for (float x = -1.0; x <= 1.0; x++)
        {
            p[i++] = GetPointPosition(id, vec2(x, y));
        }
    }

    for (int i = 0; i < 9; i++)
    {
        m += Line(gv, p[4], p[i]);
    }

    m += Line(gv, p[1], p[3]);
    m += Line(gv, p[1], p[5]);
    m += Line(gv, p[5], p[7]);
    m += Line(gv, p[7], p[3]);

    float randR = clamp(sin(iTime), 0.0, 0.3);
    resultColor = vec3(0.0);
    if (iColor.r >= 1.0)
    {
        resultColor.rgb = vec3(iColor.r + randR, .6, 0.);
    }
    else
    {
        resultColor.rgb = vec3(.0, .7, iColor.b + randR);
    }
    if (iColor.r >= 1.0 && iColor.g >= 1.0 && iColor.b >= 1.0)
    {
        resultColor.rgb = vec3(iColor.b + randR, iColor.b + randR, iColor.b + randR);
    }
    vec3 col = vec3(m) * resultColor.rgb * 0.0008;
    ;

    gl_FragColor = vec4(col, 1.0);
}