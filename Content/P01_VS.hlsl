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
    float2 canvasXY : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float4 inPos = float4(input.pos, 1.0);
    inPos.xyz *= 100.0;
    inPos.z += 25.0;
    inPos = mul(inPos, projection);
    output.pos = inPos;
    
    float aspectRatio = projection._m11 / projection._m00;
    output.canvasXY = sign(output.pos.xy) * float2(aspectRatio, 1.8); // 1.0

    return output;
}