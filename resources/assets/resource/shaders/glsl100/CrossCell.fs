#version 100

precision mediump float;
// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;
// Input uniform values
uniform vec2  iRes;
uniform float iTime;

vec3 sdgCross(in vec2 p, in vec2 b)
{
    vec2 s = sign(p);

    p = abs(p);

    vec2  q = ((p.y > p.x) ? p.yx : p.xy) - b + .1;
    float h = max(q.x, q.y);
    vec2  o = max((h < 0.0) ? vec2(b.y - b.x, 0.0) - q : q, 0.0);
    float l = length(o);

    vec3 r = (h < 0.0 && -q.x < l) ? vec3(-q.x, 0.0, 1.0) : vec3(l, o / l);
    return vec3(sign(h) * r.x, s * ((p.y > p.x) ? r.zy * .1 : r.yz));
}
void main()
{
    vec2 uv = fragTexCoord - .5;
    // uv.x *= (iRes.x / iRes.y);
    uv *= 2.0;

    // size
    vec2 si = 0.4 + 0.3 * cos(iTime + vec2(0.0, 1.0));
    if (si.x < si.y)
        si = si.yx;
    // corner radious
    float ra = .1 * (1.0 + 0.1 * sin(iTime * 1.5));

    // sdf(p) and gradient(sdf(p))
    vec3  dg = sdgCross(uv, si);
    float d  = (dg.x - ra);
    vec2  g  = dg.yz;

    // coloring
    vec3 col = (d > 0.0) ? vec3(0.0) : vec3(1.0);
    col *= 1.0 + vec3(1.0 * g, 0.1);
    col *= 1. - 1. * exp(-30.0 * abs(d));
    // col *= 1.0 + 0.5 * cos(360.0 * d);

    col *= vec3(0.0, 0.0, 1.0);
    // col         = mix(col, vec3(1.0), 1.0 - smoothstep(0.0, 0.01, abs(d)));

    float alpha = 1.0;
    if (col.r == 0.0 && col.g == 0.0 && col.b == 0.0)
        alpha = 0.0;

    gl_FragColor = vec4(col, 1.0 * alpha);
}