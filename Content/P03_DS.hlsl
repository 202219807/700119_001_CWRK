#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 elapsedTime;
	float4 resolution;
	float4 eye;
};

struct DS_OUTPUT
{
	float4 pos		: SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal	: TEXCOORD1;
};

struct QuadTessParam 
{
	float Edges[4]  : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

static float3 QuadPos[4] = 
{
	float3(-1, 1, 0),
	float3(-1, -1, 0),
	float3(1, 1, 0),
	float3(1, -1, 0)
};


float3 bumpNormal(float2 xy, float radius)
{
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

[domain("quad")]
DS_OUTPUT main(QuadTessParam input,
	float2 UV : SV_DomainLocation)
{
	DS_OUTPUT output;
	
	float3 vPos1 = (1.0 - UV.y) * QuadPos[0].xyz + UV.y * QuadPos[1].xyz;
	float3 vPos2 = (1.0 - UV.y) * QuadPos[2].xyz + UV.y * QuadPos[3].xyz;
	
	float3 uvPos = (1.0 - UV.x) * vPos1 + UV.x * vPos2;

	// Transformations	
    float radius = 5.0;  //0.3;
	float phi    = UV.y * 2 * 3.14159265358979323846;
	float theta  = UV.x * 3.14159265358979323846;

    uvPos.z -= 25.0; // 5.0;
	uvPos.x  = radius * sin(theta) * cos(phi);
	uvPos.y  = radius * sin(theta) * sin(phi);
	uvPos.z = radius; // *cos(theta * cos(elapsedTime.x * 0.5));
	
	uvPos.x -= 0.8;
	uvPos.y -= 1.0;
	// uvPos.yz *= noise(uvPos) * 2.5;  // uvPos.xyz *= noise(uvPos) * (uvPos / radius);


	// Calculate normal
    float3 dPosdx = (1.0 / resolution.x) * (uvPos + float3(1.0, 0.0, 0.0)) - (1.0 / resolution.x) * (uvPos - float3(1.0, 0.0, 0.0));
    float3 dPosdy = (1.0 / resolution.y) * (uvPos + float3(0.0, 1.0, 0.0)) - (1.0 / resolution.y) * (uvPos - float3(0.0, 1.0, 0.0));
    float3 N = normalize(cross(dPosdx, dPosdy));
    
	
	output.pos = float4(uvPos, 1);
    
    // output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
	
	// Calculate normal
    output.normal = N;
	
	// Calculate texture coordinate
	output.texCoord = UV;

	return output;

}