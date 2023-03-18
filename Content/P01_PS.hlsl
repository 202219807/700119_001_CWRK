/* Preprocessor directives for porting GLSL code to HLSL */
#define vec2 float2 
#define vec3 float3 
#define vec4 float4 
#define mat2 float2x2 
#define mat3 float3x3 

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 timer;
    float4 resolution;
};

/**
 * TODO: Specify the following basic elements involved 
 * in 3D rendering (relating to camera and lighting) in 
 * your applicationand pass them to pixel shaders as 
 * shader constants by means of constant buffer. 
 */
static float4       EYE                = float4(0, 0, 15, 1);
static const int    MAX_MARCHING_STEPS = 100;
static const float  MIN_DIST           = 0.01;
static const float  MAX_DIST           = 100.0;
static const float  EPSILON            = 0.0001;
static const float  TIME               = timer.x + 37.0;

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

float noise(in vec3 x)
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
mat2 rot(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat2(c, s, -s, c);
}

/* Minimum */
vec2 min2(vec2 a, vec2 b)
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
 */
float surfaceSDF(vec2 p)
{   
    float sum = 0.0;
    float amp = 0.2;
    float a = 0.0;
    
    for (int i = 0; i < 2; i++)
    {
        float a = noise(vec3(p + vec2(1.0, 1.0) * (TIME + 1.0) * 0.8, 1.0));
        a -= noise(vec3(p + vec2(6.0, 0.1) * TIME * 0.5, 1.0));
        
        sum += amp * a;
        
        amp *= 0.8;
        p *= 4.0;
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

float rodSDF(vec3 p, float h, float r)
{
    p.y -= clamp(p.y, 0.0, h);
    return length(p) - r;
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
    
    return min2(vec2(surface, 1.0),
           min2(vec2(floorSDF(p), 2.5),
           min2(vec2(plantsSDF(p - vec3(0.0, 0.0, 0.0)), 5.5),
           min2(vec2(bubbleSDF(pp, TIME - 0.8), 4.5),
                vec2(bubbleSDF(pp, TIME), 2.5)))));
}

/**
 * Adv. effects:
 * Caustics, God Rays and Ambient Occulsion
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

float ambientOcculsion(vec3 p, vec3 n)
{
    const float dist = 0.5;
    return smoothstep(0.0, 1.0, 1.0 - (dist - sceneSDF(p + n * dist).x));
}

/**
 * Using the gradient of the SDF, estimate the normal on the surface at point p.
 */
vec3 getNormal(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.0025;
    return normalize(e.xyy * sceneSDF(p + e.xyy).x +
					 e.yyx * sceneSDF(p + e.yyx).x +
					 e.yxy * sceneSDF(p + e.yxy).x +
					 e.xxx * sceneSDF(p + e.xxx).x);
}

/**
 * Return the normalized direction to march in from the eye point for a single pixel.
 *
 * ro: eye position
 * lookAt: lookat position
 * uv: the x,y coordinate of the pixel in the output image
 */
vec3 getRayDir(vec3 ro, vec3 lookAt, vec2 uv)
{
    vec3 forward = normalize(lookAt - ro);
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), forward));
    vec3 up = cross(forward, right);
    return normalize(forward + right * uv.x + up * uv.y);
}

void render(Ray ray, out vec4 fragColor, in vec2 fragCoord, in vec2 uv)
{
    // Camera
    ray.o = vec3(-0.4, -2.0, -6.);
    ray.o.xz = mul(ray.o.xz, rot(0.03 * sin(TIME * 0.6)));
    ray.o.y += sin(TIME * 0.2) * 0.3;
    ray.d = getRayDir(ray.o, vec3(-1.0, -2.0, -1.), uv);
    
    
    // Ray marching
    int hit = 0;  
    float d = 0.01;      // Ray distance travelled.
    float maxd = 50.0;   // Max ray distance.
    
    vec3 p;
    float outside = 1.0; // Tracks inside/outside of bubble (for refraction)
    
    for (float steps = 0.0; steps < 150.0; steps++)
    {
        p = ray.o + ray.d * d;
        vec2 h = sceneSDF(p);

        if (h.x < 0.001)
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
        
        if (d > maxd)
            break;

        d += h.x;
    }

    // Shading
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
                mat += vec3(0.1, 0.1, 0.0); // Sand
            }
            else if (hit == 5)
            {
                mat += vec3(0.0, 0.2, 0.0); // Plant
            }

            // Visual Effects
            mat += smoothstep(0.0, 1.0, (1.0 - caustic(p * 0.5)) * 0.5); // Caustics
            mat *= 0.4 + 0.6 * godLight(p, lightPos); // God light
            mat *= ambientOcculsion(p, n); // Ambient occlusion
                
            // Shadows
            vec3 lightDir = normalize(lightPos - p);
            float sha1 = max(0.0, sceneSDF(p + lightDir * 0.25).x / 0.25);
            float sha2 = max(0.0, sceneSDF(p + lightDir).x);
            mat *= clamp((sha1 + sha2) * 0.5, 0.0, 1.0);
        }
        
        // Lighting
        vec3 lightCol = vec3(1.0, 0.9, 0.8);
        vec3 lightToPoint = normalize(lightPos - p);
        
        float amb = 0.1;
        float diff = max(0.0, dot(lightToPoint, n));

        col = (amb + diff) * mat;
        
    }
    
    // Fog
    float fog = clamp(pow(d / maxd * 2.0, 1.5), 0.0, 1.0);
    col = lerp(col, deepColor, fog);
        
    
    // God rays
    col = lerp(col, vec3(0.15, 0.25, 0.3) * 12.0, cast(ray.o, ray.d, lightPos, d));
    

    // Post processing
    col = pow(col, vec3(0.4545, 0.4545, 0.4545)); // Gamma correction
    // col = vignette(col, fragCoord); // Fade screen corners 
    // col *= 1.0 - 0.05 * length(uv);
    fragColor = vec4(col, 1.0);
    
}

float4 main(PixelShaderInput input) : SV_Target
{
    // specify primary ray: 
    Ray ray;

    // set ray direction in view space 
    float dist2Imageplane = 5.0;
    float3 viewDir = float3(input.canvasXY, -dist2Imageplane);
    viewDir = normalize(viewDir);

    ray.o = float3(0, 0, 15);
    ray.d = viewDir;
    
    float4 fragColor;
    
    render(ray, fragColor, input.pos.xy, input.canvasXY);
    return fragColor;
}
