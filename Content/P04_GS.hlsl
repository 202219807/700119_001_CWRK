struct GS_INPUT
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
};

struct GS_OUTPUT
{
	float4 pos		: SV_POSITION;
	float3 color	: COLOR0;
	float2 uv		: TEXCOORD0;
};

[maxvertexcount(3)]
void main(
	triangle GS_INPUT input[3] : SV_POSITION,
	inout TriangleStream< GS_OUTPUT > OutputStream
)
{
	GS_OUTPUT output = (GS_OUTPUT)0;
	for (uint i = 0; i < 3; i++)
	{
		output.pos = input[i].pos;
		output.color = input[i].color;
		output.uv = (sign(input[i].pos.xy) + 1.0) / 2.0;

		OutputStream.Append(output);
	}
	OutputStream.RestartStrip();
}