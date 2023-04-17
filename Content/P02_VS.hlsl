#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer TimeConstantBuffer : register(b1)
{
    float elapsedTime;
    float3 padding2;
};

struct VS_INPUT
{
    float3 pos      : POSITION;
    float3 color    : COLOR0;
};

struct VS_OUTPUT
{
    float4 pos      : SV_POSITION;
    float3 color    : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 inPos = float4(input.pos, 1.0);

	// Transformations
    float r = 1.0f;
    inPos.x = r * sin(input.pos.y) * cos(input.pos.x);
    inPos.y = r * sin(input.pos.y) * sin(input.pos.x) - 2.0;
    inPos.z = r * cos(input.pos.y * sin(elapsedTime * 0.5));

    // Placement
    inPos.xyz *= 5.0;
    inPos.z -= 10.0;
    inPos.y += 10.0;
    inPos.x += 20.0;

    // Projection
    inPos = mul(inPos, view);
    inPos = mul(inPos, projection);

    output.pos = inPos;
    output.color = input.color;

    return output;
}