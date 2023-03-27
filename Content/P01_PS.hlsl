/* Preprocessor directives for porting GLSL code to HLSL */
#define vec2 float2 
#define vec3 float3 
#define vec4 float4 
#define mat2 float2x2 
#define mat3 float3x3 

#define mod  fmod
#define mix  lerp
#define atan atan2

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 timer;
    float4 resolution;
    float4 eye;
};

/**
 * TODO: Specify the following basic elements involved 
 * in 3D rendering (relating to camera and lighting) in 
 * your applicationand pass them to pixel shaders as 
 * shader constants by means of constant buffer. 
 */
static const int    MAX_MARCHING_STEPS = 200;
static const float  MIN_DIST           = 0.0;
static const float  MAX_DIST           = 100.0;
static const float  EPSILON            = 0.0001;
static const float  TIME               = timer.x;

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 canvasXY : TEXCOORD0;
};

struct Ray
{
    float3 o; // origin 
    float3 d; // direction 
};

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
    vec3 p = floor(x);
    vec3 k = frac(x);
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

/* Rotation */ 
float2x2 rot(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat2(c, s, -s, c);
}

/* Minimum */
float2 min2(float2 a, float2 b)
{
    return a.x < b.x ? a : b;
}

/* Soft minimum */
float smin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return lerp(b, a, h) - k * h * (1.0 - h);
}

/**
 * Signed distance functions for implicit modeling;
 * 
 * surface, floor, bubble and plant modeling based on https://www.shadertoy.com/view/WtfyWj
 * coral object modeling based on https://www.shadertoy.com/view/XsfGR8
 */
float surfaceSDF(vec2 p)
{   
    float sum = 0.0;
    float amp = 0.2;
    float a = 0.0;
    
    for (int i = 0; i < 2; i++)
    {
        float a = noise(vec3(p + vec2(1.0, 1.0) * (TIME + 1.0) * 0.8, 1.0));
        a -= noise(vec3(p + vec2(6.0, 0.8) * TIME * 0.5, 1.0));
        sum += amp * a;
        amp *= 0.8;
        p *= 3.0;
    }
    return clamp(0.05 + sum * 0.2, 0.0, 1.0);
}

float floorSDF(vec3 p)
{
    float bh = 0.0;
    
    // Sample noise to create terrain
    float terrainHeight = 0.0;
    float amplitude = 0.5;
    float frequency = 0.6;
    for (int i = 0; i < 8; i++)
    {
        terrainHeight += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    // Calculate the distance to the floor of the terrain
    float distToFloor = p.y + (terrainHeight * 1.13 + 2.5);
    
    return distToFloor;
}

float bubbleSDF(vec3 p, float t)
{
    float progress = pow(min(frac(t * 0.1) * 4.5, 1.0), 2.0);
    float maxDepth = 4.2;
    float depth = maxDepth * (0.8 - progress * progress);
    float r = lerp(0.01, 0.08, progress);
    
    float d = 2.0 - smoothstep(0.0, 1.0, min(progress * 5.0, 1.0)) * 0.3;
    
    return length(p + vec3(d, depth, -1.0 + 0.2 * progress * sin(progress * 10.0))) - r;
}

float rodSDF(vec3 p, float h, float r)
{
    p.y -= clamp(p.y, 0.0, h);
    return length(p) - r;
}

float plantSDF(vec3 p, float h)
{
    float r = 0.04 * -(p.y + 2.5) - 0.005 * pow(sin(p.y * 10.0), 4.0);
    p.z += sin(TIME * 0.5 + h) * pow(0.2 * (p.y + 5.6), 3.0);
    return rodSDF(p + vec3(0.0, 5.7, 0.0), 5.0 * h, r);
}

float plantsSDF(vec3 p)
{
    vec3 dd = vec3(-0.3, -0.5, -0.5);
    
    // Make multiple copies, each one displaced and rotated.
    float d = 1e10;
    for (int i = 0; i < 8; i++)
    {
        d = min(d, min(plantSDF(p, 0.0), min(plantSDF(p + dd.xyx, 5.0), plantSDF(p + dd, 3.0))));
        p.x -= 0.01;
        p.z -= 0.06;
        p.xz = mul(p.xz, rot(0.7));
    }
    
    return d;
}

float coralSDF(vec3 r) {
    vec3 zn = vec3(r.xyz);
    float rad = 0.0;
    float hit = 0.0;
    float p = 10; //12
    float d = 2.0;
    for (int i = 0; i < 4; i++)
    {

        rad = length(zn);

        if (rad > 2.0)
        {
            hit = 0.5 * log(rad) * rad / d;
        }
        else {

            float th = atan(length(zn.xy), zn.z);
            float phi = atan(zn.y, zn.x);
            float rado = pow(rad, 8.0);
            d = pow(rad, 7.0) * 7.0 * d + 1.0;


            float sint = sin(th * p);
            zn.x = rado * sint * cos(phi * p);
            zn.y = rado * sint * sin(phi * p);
            zn.z = rado * cos(th * p);
            zn += r;
        }

    }

    return hit;
}

/**
 * Signed distance function describing the scene.
 * 
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
float2 sceneSDF(vec3 p)
{
    vec3 pp = p;
    pp.xz = mul(pp.xz, rot(-.5));
    
    float surface = -p.y - surfaceSDF(p.xz);
    float t = TIME * 0.6;
    surface += (0.5 + 0.5 * (sin(p.z * 0.2 + t) + sin((p.z + p.x) * 0.1 + t * 2.0))) * 0.4;
    
    return min2(vec2(surface, 1.5),
           min2(vec2(floorSDF(p), 3.5),
           min2(vec2(plantsSDF(p - vec3(0.0, 0.0, 0.0)), 5.5),
           min2(vec2(coralSDF(p - vec3(-3.0, -2.4, -2.8)), 7.5),
           min2(vec2(plantsSDF(p - vec3(-2.5, 0.0, -1.3)), 8.5),
           min2(vec2(coralSDF(p - vec3(-3.0, -3.0, 0.0)), 6.5),
           min2(vec2(bubbleSDF(pp, TIME - 0.8), 4.5),
                vec2(bubbleSDF(pp, TIME), 2.5))))))));
}

/**
 * Adv. effects:
 * Caustics, God Rays and Ambient Occlusion
 * 
 * caustics based on https://www.shadertoy.com/view/WdByRR 
 * god rays and ambient occlusion based on https://www.shadertoy.com/view/WtfyWj
 */
float caustic(vec3 p)
{
    return abs(noise(p + fmod(TIME, 40.0) * 2.0) - noise(p + vec3(4.0, 0.0, 4.0) + fmod(TIME, 40.0) * 1.0));
}

float godLight(vec3 p, vec3 lightPos)
{
    vec3 lightDir = normalize(lightPos - p);
    vec3 sp = p + lightDir * -p.y;

    float f = 1.0 - clamp(surfaceSDF(sp.xz) * 10.0, 0.0, 1.0);
    f *= 1.0 - length(lightDir.xz);
    return smoothstep(0.2, 1.0, f * 0.7);
}

float cast(vec3 ro, vec3 rd, vec3 light, float hitDist)
{
    // March through the scene, accumulating god rays.
    vec3 p = ro;
    vec3 st = rd * hitDist / 96.0;
    float god = 0.0;
    for (int i = 0; i < 96; i++)
    {
        float distFromGodLight = 1.0 - godLight(p, light);
        god += godLight(p, light);
        p += st;
    }

    god /= 96.0;

    return smoothstep(0.0, 1.0, min(god, 1.0));
}

float ambientOcclusion(vec3 p, vec3 n)
{
    const float dist = 0.5;
    return smoothstep(0.0, 1.0, 1.0 - (dist - sceneSDF(p + n * dist).x));
}

float3 getNormal(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.0025;
    return normalize(e.xyy * sceneSDF(p + e.xyy).x +
					 e.yyx * sceneSDF(p + e.yyx).x +
					 e.yxy * sceneSDF(p + e.yxy).x +
					 e.xxx * sceneSDF(p + e.xxx).x);
}

void render(Ray ray, out vec4 fragColor, in vec2 fragCoord)
{    
    // Ray marching
    vec3 p;

    // Tracks inside and outside of bubble (for refraction)
    float outside = 1.0;    
        
    int hit = 0;  
    float d = MIN_DIST;
    for (float i = 0.0; i < MAX_MARCHING_STEPS; i++)
    {
        p = ray.o + d * ray.d;
        vec2 h = sceneSDF(p);

        if (h.x < EPSILON)
        {
            if (h.y == 4.5)
            {
                // Bubble refraction.
                ray.d = refract(ray.d, getNormal(p) * sign(outside), 1.0);
                outside *= -1.0;
                continue;
            }
            
            hit = int(h.y);
            break;
        }
        
        if (d > MAX_DIST)
            break;

        d += h.x;
    }

    // Shading, Texturing and Material details
    vec3 deepColor = vec3(0.02, 0.08, 0.2) * 0.1;
    vec3 lightPos = vec3(-1.0, 10.0, 1.0);
    vec3 col = deepColor;
    
    if (hit > 0)
    { 
        vec3 n = getNormal(p);
        vec3 mat = vec3(0.15, 0.25, 0.6);
        if (hit == 1)
        {
            // Sea
            n.y = -n.y;   
        }
        else
        {
            if (hit == 3)
            {
                // Sand
                mat += vec3(0.1, 0.1, 0.0); 
            }
            else if (hit == 6)
            {
                // Coral            
                mat += vec3(1.32, 0.35, .15); // vec3(0.255, 0.191, 0.0);       
            }
            else if (hit == 7)
            {
                // Coral     
                mat += vec3(1.12, 0.25, .15) * 0.5; // vec3(0.255, 0.191, 0.0); 
            }
            else if (hit == 5)
            {
                // Plant
                mat += vec3(0.0, 0.2, 0.0);            
            }
            else if (hit == 8)
            {
                // Plant
                mat += vec3(0.0, 0.2, 0.0);           
            }

            // Shadows and Visual Effects
            mat += smoothstep(0.0, 1.0, (1.0 - caustic(p * 0.5)) * 0.5); // Caustics
            mat *= 0.4 + 0.6 * godLight(p, lightPos);                    // God light
            mat *= ambientOcclusion(p, n);                               // Ambient occlusion
               
            vec3 lightDir = normalize(lightPos - p);
            float s1 = max(0.0, sceneSDF(p + lightDir * 0.25).x / 0.25);
            float s2 = max(0.0, sceneSDF(p + lightDir).x);
            mat *= clamp((s1 + s2) * 0.5, 0.0, 1.0);                     // Shadows
        }
        
        // Lighting
        vec3 lightCol = vec3(1.0, 0.9, 0.8);
        vec3 lightToPoint = normalize(lightPos - p);
        
        float amb = 0.1;
        float diff = max(0.0, dot(lightToPoint, n));

        col = (amb + diff) * mat;
    }
    
    // Fog
    float fog = clamp(pow(d / MAX_DIST * 2.0, 1.5), 0.0, 1.0);
    col = lerp(col, deepColor, fog);
        
    // God rays
    col = lerp(col, vec3(0.15, 0.25, 0.3) * 12.0, cast(ray.o, ray.d, lightPos, d));
    
    // Gamma correction
    col = pow(col, vec3(0.4545, 0.4545, 0.4545));                       

    fragColor = vec4(col, 1.0);
    
}

float4 main(PixelShaderInput input) : SV_Target {
    // specify primary ray 
    Ray ray;

    // set eye position
    ray.o = eye;
    //ray.o.xz = mul(ray.o.xz, rot(0.03 * sin(TIME * 0.6)));
    //ray.o.y += sin(TIME * 0.2) * 0.3;

    // set ray direction in view space 
    float dist2Imageplane = 5.0;
    float3 viewDir = float3(input.canvasXY, -dist2Imageplane);
    viewDir = normalize(viewDir);

    // transform viewDir using the inverse view matrix
    float4x4 viewTrans = transpose(view);
    ray.d = viewDir.x * viewTrans._11_12_13 + viewDir.y * viewTrans._21_22_23
        + viewDir.z * viewTrans._31_32_33;
    
    float4 fragColor;
    
    render(ray, fragColor, input.pos.xy);
    return fragColor;
}
