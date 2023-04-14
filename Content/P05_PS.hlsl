#include "MathUtils.hlsli"

cbuffer TimeConstantBuffer : register(b0)
{
    float time;
    float3 padding2;
}

cbuffer CameraConstantBuffer : register(b1)
{
    float3 camera;
    float  padding;
}

struct PS_INPUT
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
	float2 uv		: TEXCOORD0;
};

float3 bumpNormal(float2 xy)
{
    static const float radius = 1.0;
    float3 N = float3(0.0, 0.0, 1.0);
    
    //map xy to [-1, 1]x[-1, 1]:
    
    float2 st = 2.0 * frac(xy) - 1.0;
    float r2 = radius * radius - dot(st, st);
    
    if (r2 > 0.0)
    {
        N.xy = st / sqrt(r2);
    }
    return normalize(N);
}

//dont delete
float4 main(PS_INPUT input) : SV_Target
{

	// Return the final color for the pixel
	if (input.color.x == 0.0 && input.color.y == 0.0 && input.color.z == 0.0) {
		discard;
	}

	// Lighting
    float shininess = 20.0;
    float3 K_a = float3(1.0, 1.0, 1.0);
    float3 K_d = float3(0.2, 0.2, 0.2);
    float3 K_s = float3(1.0, 1.0, 1.0);

    float3 lightPos = float3(0.0, -3.5, -16.0);
    float3 lightIntensity = float3(1.0, 1.0, 1.0);
    float3 lightDir = normalize(lightPos - input.pos.xyz);

    //float BumpDensity = 2.5;
    //float2 xy = float2(BumpDensity * input.uv.x, BumpDensity * input.uv.y);
    //float3 N = bumpNormal(xy);
    float3 N = float3(0.0, 0.0, 1.0);
    

    
	// Ambient light
    float3 ambientLight = K_a * input.color.rgb; //input.color.rgb;
	
    // Diffuse light
    float  diffuse = max(dot(N, lightDir), 0.0);
    float3 diffuseColor = lightIntensity * K_d * diffuse * N;

    // Specular light
    float3 viewDir = normalize(-input.pos.xyz);
    
    float3 reflectDir = reflect(-lightDir, N);
    
    float  specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float3 specularColor = lightIntensity * K_s * specular;

    float4 finalColor = float4(ambientLight + diffuseColor + specularColor, 1.0);
    return finalColor;
	
}








































///**
// * Phong Illumination:
// * Based on https://www.shadertoy.com/view/lt33z7
// */  

///**
// * Lighting contribution of a single point light source via Phong illumination.
// * 
// * The float3 returned is the RGB color of the light's contribution.
// *
// * k_a: Ambient color
// * k_d: Diffuse color
// * k_s: Specular color
// * alpha: Shininess coefficient
// * p: position of point being lit
// * eye: the position of the camera
// * lightPos: the position of the light
// * lightIntensity: color/intensity of the light
// *
// * See https://en.wikipedia.org/wiki/Phong_reflection_model#Description
// */
//float3 PhongContribForLight(float3 k_d, float3 k_s, float alpha, float3 p, float3 eye, float3 lightPos, float3 lightIntensity, float3 normal)
//{
//    float3 N = normal;
//    float3 L = normalize(lightPos - p);
//    float3 V = normalize(eye - p);
//    float3 R = normalize(reflect(-L, N));
    
//    float dotLN = dot(L, N);
//    float dotRV = dot(R, V);
    
//    if (dotLN < 0.0)
//    {
//        // Light not visible from this point on the surface
//        return float3(0.0, 0.0, 0.0);
//    }
    
//    if (dotRV < 0.0)
//    {
//        // Light reflection in opposite direction as viewer, apply only diffuse
//        // component
//        return lightIntensity * (k_d * dotLN);
//    }
//    return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
//}
///**
// * Lighting via Phong illumination.
// * 
// * The float3 returned is the RGB color of that point after lighting is applied.
// * k_a: Ambient color
// * k_d: Diffuse color
// * k_s: Specular color
// * alpha: Shininess coefficient
// * p: position of point being lit
// * eye: the position of the camera
// *
// * See https://en.wikipedia.org/wiki/Phong_reflection_model#Description
// */
//float3 PhongIllumination(float3 k_a, float3 k_d, float3 k_s, float alpha, float3 p, float3 eye, float3 normal)
//{
//    const float3 ambientLight = 0.9 * float3(0.5, 0.5, 0.5);
//    float3 color = ambientLight * k_a;
//    float3 lightPos = float3(0.0, 4.0, -16.0); // Refactor from buffer
//    float3 lightIntensity = float3(0.1, 0.1, 0.1);
//    color += PhongContribForLight(k_d, k_s, alpha, p, eye, lightPos, lightIntensity, normal);
//    return color;
//}

//float4 main(PS_INPUT input) : SV_Target
//{
//    float3 pixelColor;
    
//    // Lighting
//    float shininess = 10.0;
//    float3 K_a = float3(1.0, 1.0, 1.0);
//    float3 K_d = float3(0.2, 0.2, 0.2);
//    float3 K_s = float3(1.0, 1.0, 1.0);
    
//    float3 N = float3(1.0, 0.0, 0.0);
    
//    pixelColor = PhongIllumination(K_a, K_d, K_s, shininess, input.pos.xyz, camera, N) + input.color.rgb;
    
//    //return float4(input.color, 1.0);
//     return float4(pixelColor, 1.0);


//}

