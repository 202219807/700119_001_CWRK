/**
* Mathematics utilities:
* Noise functions, Transformations and Soft blending
*/

/**
* Noise function sampled from:
* https://www.shadertoy.com/view/WdByRR
*/

float hash(float n)
{
    return frac(sin(n) * 43758.5453);
}

float noise(in float3 x)
{
    float3 p = floor(x);
    float3 k = frac(x);
    k = k * k * (3.0 - 2.0 * k);

    float n = p.x + p.y * 57.0 + p.z * 113.0;
    float a = hash(n);
    float b = hash(n + 1.0);
    float c = hash(n + 57.0);
    float d = hash(n + 58.0);

    float e = hash(n + 113.0);
    float f = hash(n + 114.0);
    float g = hash(n + 170.0);
    float h = hash(n + 171.0);

    float res = lerp(lerp(lerp(a, b, k.x), lerp(c, d, k.x), k.y),
        lerp(lerp(e, f, k.x), lerp(g, h, k.x), k.y),
        k.z);

    return res;
}
