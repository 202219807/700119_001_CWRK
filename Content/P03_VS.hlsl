cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 timer;
	float4 resolution;
};

static float time = timer.x;
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

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float3 inPos = input.pos;

	float r = 1.0f;
	inPos.x = r * sin(input.pos.y) * cos(input.pos.x);
	inPos.y = r * sin(input.pos.y) * sin(input.pos.x) - 2.0;
	inPos.z = r * cos(input.pos.y * sin(time * 0.5));
	
	float4 pos = float4(inPos, 1.0f);

	pos = mul(pos, model);
	// pos = mul(float4(20 * pos.xy, pos.zw), model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.color = input.color;

	return output;
}