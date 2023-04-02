cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct VS_INPUT
{
    float3 pos      : POSITION;
    float3 color    : COLOR0;
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
    
    // Transformations
    inPos.xyz *= 100.0; // 10.0
    inPos.z += 25.0;
    
    // inPos = float4(sign(inPos.xy), 0, 1);
    //
    // or
    //
    // inPos = mul(float4(10 * inPos.xy, inPos.zw), model);
    // inPos = mul(inPos, model);

    inPos = mul(inPos, view);
    inPos = mul(inPos, projection);
    output.pos = inPos;
    
    float aspectRatio = projection._m11 / projection._m00;
    output.canvasXY = sign(output.pos.xy) * float2(aspectRatio, 1.8); // 1.0

    return output;
}