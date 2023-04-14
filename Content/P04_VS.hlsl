struct VS_INPUT
{
    float3 pos : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    output.pos = float4(input.pos, 1.0);
    output.pos.z -= 20;
    output.pos.y -= 4;
    output.pos.x -= 20;
    output.color = float4(input.color, 1.0);
    output.normal = input.normal;

    return output;
}