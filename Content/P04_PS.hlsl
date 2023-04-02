struct PS_INPUT
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
	float2 uv		: TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = float4(input.color, 1.0f);
	return color;
}