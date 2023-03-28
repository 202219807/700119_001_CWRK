cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 timer;
	float4 resolution;
	float4 eye;
};

struct GeometryShaderInput
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
};

struct PixelShaderInput
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
	float2 uv		: TEXCOORD0;
};

[maxvertexcount(6)]
void main(point GeometryShaderInput input[1], inout
	TriangleStream<PixelShaderInput> OutputStream)
{
	PixelShaderInput output = (PixelShaderInput)0;

	float4 vPos = input[0].pos;
	vPos = mul(vPos, model);
	vPos = mul(vPos, view);

	static const float3 g_positions[4] =
	{
		float3(-1, 1, 0),
		float3(-1, -1, 0),
		float3(1, 1, 0),
		float3(1, -1, 0),
	};

	// triangle 1 

	float quadSize = 0.15;

	// vertex 1:  
	output.pos = vPos + float4(quadSize * g_positions[0], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	// vertex 2:  
	output.pos = vPos + float4(quadSize * g_positions[1], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	// vertex 3:  
	output.pos = vPos + float4(quadSize * g_positions[2], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	OutputStream.RestartStrip();

	//triangle 2 

	quadSize = 0.1;

	// vertex 1:  
	output.pos = vPos + float4(quadSize * g_positions[1], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	// vertex 2:  
	output.pos = vPos + float4(quadSize * g_positions[2], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	// vertex 3:  
	output.pos = vPos + float4(quadSize * g_positions[3], 0.0);
	output.pos = mul(output.pos, projection);

	output.color = input[0].color;
	output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;

	OutputStream.Append(output);

	OutputStream.RestartStrip();

}