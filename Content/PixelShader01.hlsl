/* Resources:
 * https://www.shadertoy.com/view/wtKSzD
 * https://www.youtube.com/watch?v=l-07BXzNdPw
 */
#define PI 3.141592
#define MAX_STEPS 500
#define MAX_DIST 200.0
#define SURF_DIST 0.001

#define GROUND 0
#define WATER 1

/* Skewing and "unskewing" constants: 
 * As decribed here: https://en.wikipedia.org/wiki/Simplex_noise
 */
#define S (sqrt(.75)-.5)
#define G (.5-rsqrt(12.))

/** 
 * Preprocessor directives for porting GLSL code to HLSL
 */
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
    float4 time;
    float4 resolution;
};

static float amplitude = 0.5;
static float frequency = 1.0;
static float TIME = time.x * frequency;

//static float TIME = time.x;

static float4 Eye = float4(0, 0, 15, 1);

struct Ray
{
    float3 o; // origin 
    float3 d; // direction 
};

struct Obj
{
    int type;
    float dist;
    float3 hitPos;
};

struct VS_Canvas
{
    float4 Position : SV_POSITION;
    float2 canvasXY : TEXCOORD0;
};

// caustic effect
vec2 n22(vec2 p)
{
    return frac(sin(vec2(dot(p, vec2(123.4, 748.6)), dot(p, vec2(547.3, 659.3)))) * 5232.85324);
}

float voronoi(vec2 p)
{
    // divide the screen into a grid
    p *= 3.;
    
    // this is the id of a grid cell. x and y go from -3 to 3.
    vec2 id = floor(p);
    
    // coordinates inside a grid cell
    vec2 gv = frac(p);
    
    float minDist = 9.;
    
    // loop through 9 grid cells around the point and check 
    // which is the closest cell
    for (float y = -1.; y <= 1.; y++)
    {
        for (float x = -1.; x <= 1.; x++)
        {
            vec2 offset = vec2(x, y);
            vec2 n = n22(id + offset);
            // multiply because gv goes from -0.5 to .0.5
            vec2 p = .5 + 0.5 * sin(n * TIME * 4.);
      
            vec2 r = offset + p - gv;
            float d = dot(r, r);
            
            if (d < minDist)
            {
                minDist = d;
            }
        }
    }
    
    return minDist;
}

// creates fake caustics with voronoi patterns and fBMs
float caustics(vec2 p)
{
    float v = 0.0;
    float a = 0.4;
    for (int i = 0; i < 3; i++)
    {
        v += voronoi(p) * a;
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

// create god rays
float godRays(vec3 rayDir)
{
    float y = rayDir.y * 0.5 + 0.1;
    
    float a = atan2(rayDir.x, rayDir.z);
    
    float rays = sin(a * 28. + TIME) * sin(a * 27. - TIME) * sin(a * 22.);
    rays = max(0., rays);
    
    // cut the lower part of the rays
    rays *= smoothstep(0.1, 1.0, y);
    
    // decrease brightness
    rays *= smoothstep(0.8, 0.2, y);
    
    return rays + 1.0;
}

//2D signed hash function:
vec2 hash2(vec2 P)
{
    return 1. - 2. * frac(cos(P.x * vec2(91.52, -74.27) + P.y * vec2(-39.07, 09.78)) * 939.24);
}

//2D simplex gradient noise.
float simplex(vec2 P)
{
    //calculate simplex cells.
    vec2 N = P + S * (P.x + P.y);
    vec2 F = floor(N);
    vec2 T = vec2(1, 0) + vec2(-1, 1) * step(N.x - F.x, N.y - F.y);
    
    //distance to the nearest cells.
    vec2 A = F - G * (F.x + F.y) - P;
    vec2 B = F + T - G * (F.x + F.y) - G - P;
    vec2 C = F + 1. - G * (F.x + F.y) - G - G - P;
    
    //calculate weights and apply quintic smoothing.
    vec3 I = max(.5 - vec3(dot(A, A), dot(B, B), dot(C, C)), 0.);
    I = I * I * I * (6. * I * I, -15. * I + 10.);
    I /= dot(I, vec3(1.0, 1.0, 1.0));
    
    //add up the gradients.
    return .5 + (dot(hash2(F), A) * I.x +
        	   dot(hash2(F + T), B) * I.y +
        	   dot(hash2(F + 1.), C) * I.z);
}

/*
 * Fractal Brownian Motion :
 * Adds different iterations of noise (octaves) 
 * by incrementing the frequencies in regular steps and decrease the amplitude 
 * of the noise. It makes the noise more granular and add more fine details.
*/
float fbm(in vec2 uv, int octaves, float octaveWeight)
{
    
    float h = 0.; // height
    float m = 0.4; // octave multiplier
    
    for (int i = 0; i < octaves; i++)
    {
        h += octaveWeight * simplex(uv * m);
        octaveWeight *= .4;
        m *= 2.0;
    }
    return h;
}

Obj getDist(vec3 p)
{
    // distance to the ground
    float height = fbm(vec2(p.x, p.z), 2, 0.4);
    height = fbm(vec2(p.x, p.z), 6, 1.5);
    float planeDist = p.y - height - 3.;
   
    Obj obj;
    obj.type = GROUND;
    obj.dist = planeDist;
    obj.hitPos = p;

    return obj;
}

vec3 getNormal(vec3 p)
{
    float d = getDist(p).dist;
    
    // use offset samples to compute normal
    vec2 e = vec2(.01, 0.);
    vec3 n = d - vec3(
        getDist(p - e.xyy).dist,
        getDist(p - e.yxy).dist,
        getDist(p - e.yyx).dist);
    
    return normalize(n);
}

float getLight(vec3 p, vec3 lightPos)
{
    
    // compute the light vector and its normal   
    lightPos.xz += vec2(sin(TIME * 0.5), cos(TIME * 0.5)) * 1.4;
    vec3 l = normalize(lightPos - p);
    vec3 n = getNormal(p);
    
    // calculate diffuse lightning
    // dot product gives values between -1 to 1 hence the range
    // is clamped to 0 to 1
    float dif = clamp(dot(n, l), 0.0, 1.0);
    
    return dif;
}

// perform ray marching
Obj rayMarch(Ray ray)
{
    float t = 0.0;
    
    Obj hitObj;

    hitObj.type = WATER;
    hitObj.dist = t;
    hitObj.hitPos = ray.o + ray.d;
    
    for (int i = 0; i < MAX_STEPS; i++)
    {
    	
        vec3 p = ray.o + ray.d * t;
        hitObj = getDist(p);
        t += hitObj.dist;
        
        // the ray has marched far enough but hit nothing. 
        // Render the pixel as a part of the sky.
        if (t > MAX_DIST)
        {
            hitObj.type = WATER;
            hitObj.dist = t;
            hitObj.hitPos = p;
            break;
        }

        // the ray has marched close enough to an object
        if (abs(hitObj.dist) < SURF_DIST * (1.0 + t))
        {
            hitObj.type = hitObj.type;
            hitObj.dist = t;
            hitObj.hitPos = p;
            break;
        }
        
        hitObj.dist = t;
    }
    
    return hitObj;
}

// calculates the intersection of the given plane and ray
vec3 intersectPlane(vec3 ori, vec3 dir, vec3 planePos)
{
    
    vec3 planeNormal = vec3(0, 1, 0);
    
    float a = dot(dir, planeNormal);
   
    float distToPlane = dot(ori - planePos, planeNormal);
    vec3 intersectPt = ori + dir * abs(distToPlane / a);
    
    return intersectPt;
}

mat3 lookAt(vec3 origin, vec3 target, float roll)
{ 
    vec3 rr = vec3(sin(roll), cos(roll), 0.0);
    vec3 ww = normalize(target - origin);
    vec3 uu = normalize(cross(ww, rr));
    vec3 vv = normalize(cross(uu, ww));

    return mat3(uu, vv, ww);
}

void render(Ray ray, out vec4 fragColor, in vec2 fragCoord, in vec2 uv)
{
    vec3 GROUND_COLOR = vec3(0.45, 0.45, 0.45);
    vec3 HORIZON_COLOR = vec3(0.0, 0.045, 0.2);
    vec3 LIGHT_DIR = vec3(0.0, 10.0, -10.0);
    
    // camera
    vec3 camPos = vec3(3.0 * sin(TIME * 0.25), 7.0, -1. * TIME);
    camPos = vec3(10.0 * sin(TIME * 0.2), 7., -9.);
    
    vec3 camTarg = vec3(1., 4., 4.);
    mat3 camMat = lookAt(camPos, camTarg, 0.0);
    
    // update primary ray
    ray.o = camPos;
    ray.d = normalize(mul(camMat, vec3(uv.xy, 1.0)));
    // ray.d = normalize(mul(camMat, vec3(uv.xy, 1.0)));
    // ray.d = normalize(float3(
    //                    dot(camMat[0], float3(uv.xy, 1.0)), 
    //                    dot(camMat[1], float3(uv.xy, 1.0)),
    //                    dot(camMat[2], float3(uv.xy, 1.0))
    //                ));
    
    vec3 col = vec3(1.0, 1.0, 1.0);
    
    // ray marching
    Obj hitObj = rayMarch(ray);
   	
    if (hitObj.type == WATER)
    {
        // to tune the horizon light
        col = lerp(vec3(0.01, 0.01, 0.01),
            HORIZON_COLOR, 
            pow(1.0 - pow(ray.d.y, 4.), 0.1)
        );
    }
    else if (hitObj.type == GROUND)
    {
        float dif = getLight(hitObj.hitPos, LIGHT_DIR) * hitObj.hitPos.y * 0.26;
        col = vec3(dif, dif, dif);
                     
        float caust = caustics(uv) * 3.5;
        col *= clamp(caust, 0.5, 1.);
        
        // use beer's law for calculating attenuation
		vec3 attenuation = exp(-vec3(0.3, 0.15, 0.08) * hitObj.dist);
        col = lerp(HORIZON_COLOR, col, attenuation);
    }
    
    float godRay = godRays(ray.d);
  
    vec3 lightColor = lerp(vec3(0.15, 0.15, 0.20), col, 0.5 - uv.y);
    
    vec3 finalColor = lerp(col, lightColor, godRay);
    col = pow(finalColor, vec3(.454545, .454545, .454545));

    fragColor = vec4(col, 1.0);
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
 
    render(eyeray, fragColor, In.Position.xy, In.canvasXY);
  
    return fragColor;
}


