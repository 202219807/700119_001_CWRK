cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 resolution;
	float4 timer;
	float4 eye;
};

struct VertexShaderOutput
{
	float4 pos		: SV_POSITION;
};

struct QuadTessParam {
	float Edges[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

static float3 QuadPos[4] = {
	float3(-1, 1, 0),
	float3(-1, -1, 0),
	float3(1, 1, 0),
	float3(1, -1, 0)
};

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

	float res = lerp(lerp(lerp(a, b, k.x), lerp(c, d, k.x), k.y), lerp(lerp(e, f, k.x), lerp(g, h, k.x), k.y), k.z);
	return res;
}

[domain("quad")]
VertexShaderOutput main(QuadTessParam input,
	float2 UV : SV_DomainLocation)
{
	VertexShaderOutput output;
	
	float3 vPos1 = (1.0 - UV.y) * QuadPos[0].xyz
		+ UV.y * QuadPos[1].xyz;
	float3 vPos2 = (1.0 - UV.y) * QuadPos[2].xyz
		+ UV.y * QuadPos[3].xyz;
	
	float3 uvPos = (1.0 - UV.x) * vPos1 + UV.x * vPos2;
	uvPos.y -= 2.0;
	uvPos.z -= 2.0;

	float3 spherePos;
	float radius = 0.3;
	float phi = UV.y * 2 * 3.14159265358979323846;
	float theta = UV.x * 3.14159265358979323846;

	spherePos.x = radius * sin(theta) * cos(phi);
	spherePos.y = radius * sin(theta) * sin(phi);
	spherePos.z = radius * cos(theta);

	output.pos = float4(spherePos, 1);
	output.pos.xyz += noise(spherePos) * (spherePos / radius);


	return output;





}