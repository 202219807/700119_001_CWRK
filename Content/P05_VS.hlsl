#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer CameraTrackingBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
}

cbuffer TimeConstantBuffer : register(b2)
{
    float iTime;
    float3 padding2;
}

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
    output.pos = float4(input.pos, 1.0);
    output.color = Gradient(output.pos.xyz);

    return output;
}