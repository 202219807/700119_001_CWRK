#define Control_Points 4

struct HS_INPUT
{
	float4 pos		: SV_POSITION;
};

struct HS_OUTPUT
{
	float4 pos		: SV_POSITION;
};

struct QuadTessFactors
{
	float Edges[4]	: SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;
};

QuadTessFactors CalcHSPatchConstants(
	InputPatch<HS_INPUT, Control_Points> ip,
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
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_OUTPUT main(
	InputPatch<HS_INPUT, Control_Points> patch,
	uint i : SV_OutputControlPointID)
{
	HS_OUTPUT Output;
	Output.pos = patch[i].pos;
	return Output;
}
