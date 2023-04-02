cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct VS_INPUT
{
    float3 pos      : POSITION;
    float3 color    : COLOR;
};

struct VS_OUTPUT
{
    float4 pos      : SV_POSITION;
    float3 color    : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transformations

    float4 inPos = float4(input.pos, 1.0);
    inPos.xyz *= 5.0;
    inPos.z -= 10.0;
    //inPos = mul(inPos, model);
    inPos = mul(inPos, view);
    inPos = mul(inPos, projection);

    output.pos = inPos;
    output.color = input.color;

    return output;
}