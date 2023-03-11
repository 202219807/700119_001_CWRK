// Preprocessor directives for porting GLSL code to HLSL
#define vec2 float2 
#define vec3 float3 
#define vec4 float4 
#define mat2 float2x2 
#define mat3 float3x3 

// TODO: Specify the following basic elements involved 
// in 3D rendering (relating to camera and lighting) in 
// your applicationand pass them to pixel shaders as 
// shader constants by means of constant buffer. 

static float  NearPlane = 1.0;
static float  FarPlane = 1000.0;
static float3 LightPos = float3(0, 100, 0);
static float4 LightColor = float4(1, 1, 1, 1);
static float4 BackgroundColor = float4(0.1, 0.2, 0.3, 1);
static float4 Eye = float4(0, 0, 15, 1);


static const int   MAX_MARCHING_STEPS = 255;
static const float MIN_DIST = 0.0;
static const float MAX_DIST = 100.0;
static const float EPSILON = 0.0001;

struct Ray
{
    float3 o; // origin 
    float3 d; // direction 
};

struct VS_Canvas
{
    float4 Position : SV_POSITION;
    float2 canvasXY : TEXCOORD0;
};

/**
 * Signed distance function for a sphere centered at the origin with radius 1.0;
 */
float sphereSDF(vec3 samplePoint)
{
    return length(samplePoint) - 1.0;
}

/**
 * Signed distance function describing the scene.
 * 
 * Absolute value of the return value indicates the distance to the surface.
 * Sign indicates whether the point is inside or outside the surface,
 * negative indicating inside.
 */
float sceneSDF(vec3 samplePoint)
{
    return sphereSDF(samplePoint);
}

/**
 * Return the shortest distance from the eyepoint to the scene surface along
 * the marching direction. If no part of the surface is found between start and end,
 * return end.
 * 
 * eye: the eye point, acting as the origin of the ray
 * marchingDirection: the normalized direction to march in
 * start: the starting distance away from the eye
 * end: the max distance away from the ey to march before giving up
 */
float shortestDistanceToSurface(vec3 eye, vec3 marchingDirection, float start, float end)
{
    float depth = start;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++)
    {
        float dist = sceneSDF(eye + depth * marchingDirection);
        if (dist < EPSILON)
        {
            return depth;
        }
        depth += dist;
        if (depth >= end)
        {
            return end;
        }
    }
    return end;
}



float hash(float n)
{
    return frac(sin(n) * 43758.5453);
}

//Hash from iq
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

void mainImage(Ray ray, out vec4 fragColor, in vec2 fragCoord)
{   
    
    float dist = shortestDistanceToSurface(ray.o, ray.d, MIN_DIST, MAX_DIST);
    
    if (dist > MAX_DIST - EPSILON)
    {
        // Didn't hit anything
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }
    
     // Procedurally generate the texture
    float pattern = (sin(ray.o.x * 10.0) + cos(ray.o.y * 10.0) + sin(ray.o.z * 10.0)) * 0.3333;
    
    vec4 color = vec4(noise(pattern*2), noise(pattern*3), noise(pattern*4), 1.0);
    
    // Apply lighting
    vec3 pointOnSurface = ray.o + dist * ray.d;
    vec3 surfaceNormal = normalize(vec3(
        sceneSDF(pointOnSurface + vec3(EPSILON, 0, 0)) - sceneSDF(pointOnSurface - vec3(EPSILON, 0, 0)),
        sceneSDF(pointOnSurface + vec3(0, EPSILON, 0)) - sceneSDF(pointOnSurface - vec3(0, EPSILON, 0)),
        sceneSDF(pointOnSurface + vec3(0, 0, EPSILON)) - sceneSDF(pointOnSurface - vec3(0, 0, EPSILON))
    ));
    
    vec3 lightDir = normalize(LightPos - pointOnSurface);
    float diffuse = max(dot(surfaceNormal, lightDir), 0.0);
    vec4 diffuseColor = LightColor * diffuse;
    
    fragColor = color * diffuseColor + BackgroundColor;
}

float4 main(VS_Canvas In) : SV_Target
{
    // specify primary ray: 
    Ray eyeray;

    // set ray direction in view space 
    float dist2Imageplane = 5.0;
    float3 viewDir = float3(In.canvasXY, -dist2Imageplane);
    viewDir = normalize(viewDir);
    
    eyeray.o = Eye.xyz;
    eyeray.d = viewDir;
    
    float4 fragColor;
 
    mainImage(eyeray, fragColor, In.Position.xy);
  
    return fragColor;
}
