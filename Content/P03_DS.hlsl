#include "Utilities.hlsli"

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
	
	float3 vPos1 = (1.0 - UV.y) * QuadPos[0].xyz
		+ UV.y * QuadPos[1].xyz;
	float3 vPos2 = (1.0 - UV.y) * QuadPos[2].xyz
		+ UV.y * QuadPos[3].xyz;
	
	float3 uvPos = (1.0 - UV.x) * vPos1 + UV.x * vPos2;
	uvPos.z -= 2.0;

	// Transformations
	float3 spherePos;
	
	float radius = 0.3;
	float phi    = UV.y * 2 * 3.14159265358979323846;
	float theta  = UV.x * 3.14159265358979323846;

	spherePos.x  = radius * sin(theta) * cos(phi);
	spherePos.y  = radius * sin(theta) * sin(phi);
	spherePos.z  = radius * cos(theta * cos(elapsedTime.x * 0.5));

	output.pos = float4(spherePos, 1);
	output.pos.xyz += noise(spherePos) * (spherePos / radius);
	output.pos.x -= 0.8;
	output.pos.y -= 1.0;
	output.pos.yz *= noise(output.pos.xyz) * 2.5;
	
	return output;

}