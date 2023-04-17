/**
 * Mathematics utilities:
 * Noise functions, Transformations and Soft blending
 */

/* Rotation */ 
float2x2 rot(float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2x2(c, s, -s, c);
}

/* Minimum */
float2 min(float2 a, float2 b)
{
    return a.x < b.x ? a : b;
}

/* Maximum */
float2 max(float2 a, float2 b)
{
    return a.x < b.x ? b : a;
}

/* Soft minimum */
float softMin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return lerp(b, a, h) - k * h * (1.0 - h);
}

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

float hash2(float2 grid) {
    float h = dot(grid, float2 (127.1, 311.7));
    return hash(h);
}

float perlin(in float2 p)
{
    float n1, n2, n3, n4;
    float2 grid = floor(p);
    float2 f = frac(p);
    float2 uv = f * f * (3.0 - 2.0 * f);
    n1 = hash2(grid + float2(0.0, 0.0)); n2 = hash2(grid + float2(1.0, 0.0));
    n3 = hash2(grid + float2(0.0, 1.0)); n4 = hash2(grid + float2(1.0, 1.0));
    n1 = lerp(n1, n2, uv.x); n2 = lerp(n3, n4, uv.x);
    n1 = lerp(n1, n2, uv.y);
    return n1; //2*(2.0*n1 -1.0);
}

/* Gradient */
float3 Gradient(float3 position)
{
    // Define the start and end colors of the gradient
    float3 startColor = float3(0.0, 1.0, 1.0); // Blue
    float3 endColor = float3(1.0, 0.0, 0.0);   // Yellow

    // Calculate the position's distance from the origin
    float distance = length(position);

    // Normalize the position vector to get a value between 0 and 1
    float t = saturate(dot(normalize(position), normalize(float3(-1.0, 1.0, 0.0))));

    // Add variation to the colors
    float3 colorVariationVector = sin(position * 0.6);
    startColor += colorVariationVector;
    endColor += colorVariationVector;

    // Interpolate between the start and end colors based on the distance
    return lerp(startColor, endColor, t);
}
