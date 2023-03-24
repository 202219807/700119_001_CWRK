cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 timer;
	float4 resolution;
	float4 eye;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

static float time = timer.x;

float2x2 rot(float a)
{
	float c = cos(a);
	float s = sin(a);
	return float2x2(c, s, -s, c);
}

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 inPos = float4(input.pos, 1.0);

	// Transformations
	float r = 1.0f;
	inPos.x = r * sin(input.pos.y) * cos(input.pos.x);
	inPos.y = r * sin(input.pos.y) * sin(input.pos.x) - 2.0;
	inPos.z = r * cos(input.pos.y * sin(time * 0.5));
	
	inPos.xyz *= 8.5;	
	inPos.z -= 10.0;
	inPos.x += 20.0;
	inPos.y += 5.0;

	inPos = mul(inPos, model);
	inPos = mul(inPos, view);
	inPos = mul(inPos, projection);
	output.pos = inPos;

	output.color = float3(0.96, 0.48, 0.10); // input.color;

	return output;
}