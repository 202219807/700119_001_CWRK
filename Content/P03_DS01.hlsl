#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer CameraConstantBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
}

cbuffer TimeConstantBuffer : register(b2)
{
    float time;
    float3 padding2;
}

cbuffer NoiseConstantBuffer : register(b3)
{
    float noiseStrength;
    float3 padding4;
}

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

	uvPos.x  = radius * sin(theta) * cos(phi);
	uvPos.y  = radius * sin(theta) * sin(phi);
    uvPos.z = radius * cos(theta);	
    uvPos.xz += noise(uvPos) * 2.5 * noiseStrength;
    
	output.pos = float4(uvPos, 1);
    
	// output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
	
	// Calculate normal
    output.normal = normalize(float3(-0.5, 3.0, 4.0) - uvPos.xyz);
	
	// Calculate texture coordinate
	output.texCoord = UV;

	return output;
}