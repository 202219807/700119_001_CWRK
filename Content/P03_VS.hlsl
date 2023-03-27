struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
};

VertexShaderOutput main()
{
	VertexShaderOutput output;
	output.pos = float4(0.0,0.0,0.0,1.0);
	return output;
}