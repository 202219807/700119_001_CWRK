struct VertexShaderOutput
{
	float4 pos		: SV_POSITION;
};

struct QuadTessFactors
{
	float Edges[4]	: SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;
};

#define CP 4

QuadTessFactors CalcHSPatchConstants(
	InputPatch<VertexShaderOutput, CP> ip,
	uint PatchID : SV_PrimitiveID)
{
	QuadTessFactors Output;
	float TessAmount = 50;
	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
	Output.Inside[0] = Output.Inside[1] = TessAmount;
	return Output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
VertexShaderOutput main(
	InputPatch<VertexShaderOutput, CP> patch,
	uint i : SV_OutputControlPointID)
{
	VertexShaderOutput Output;

	Output.pos = patch[i].pos;

	return Output;
}
