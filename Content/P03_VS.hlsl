struct VS_OUTPUT
{
	float4 pos		: SV_POSITION;
};

VS_OUTPUT main()
{
	VS_OUTPUT output;
	output.pos = float4(0.0, 0.0, 0.0, 1.0);
	return output;
}