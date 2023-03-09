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

void mainImage(Ray ray, out vec4 fragColor, in vec2 fragCoord)
{   
    float dist = shortestDistanceToSurface(ray.o, ray.d, MIN_DIST, MAX_DIST);
    
    if (dist > MAX_DIST - EPSILON)
    {
        // Didn't hit anything
        fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }
    
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
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


