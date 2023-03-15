cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 timer;
    float4 resolution;
};

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 canvasXY : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    float4 inPos = float4(input.pos, 1.0);

    output.pos = mul(float4(20 * inPos.xy, inPos.zw), model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    float aspectRatio = projection._m11 / projection._m00;
    output.canvasXY = sign(output.pos.xy) * float2(aspectRatio, 1.8);

    return output;
}
