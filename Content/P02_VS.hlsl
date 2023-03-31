cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 elapsedTime;
	float4 resolution;
	float4 eye;
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	float4 inPos = float4(input.pos, 1.0);

	// Transformations
	float r = 1.0f;
	inPos.x = r * sin(input.pos.y) * cos(input.pos.x);
	inPos.y = r * sin(input.pos.y) * sin(input.pos.x) - 2.0;
	inPos.z = r * cos(input.pos.y  * sin(elapsedTime.x * 0.5));

	inPos.xyz *= 5.0;
	inPos.z -= 10.0;
	inPos.y -= 2.0;
	inPos.x += 20.0;

	inPos = mul(inPos, view);
	inPos = mul(inPos, projection);

	output.pos = inPos;
	output.color = input.color; // float3(0.96, 0.48, 0.10); 

	return output;
}