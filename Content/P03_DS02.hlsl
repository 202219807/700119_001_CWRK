#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer CameraConstantBuffer : register(b1)
{
    float3 cameraPosition;
    float padding;
}

cbuffer TimeConstantBuffer : register(b2)
{
    float time;
    float3 padding2;
}

cbuffer NoiseConstantBuffer : register(b3)
{
    float noiseStrength;
    float3 padding4;
}

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

struct QuadTessParam
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

static float3 QuadPos[4] =
{
    float3(-1, 1, 0),
	float3(-1, -1, 0),
	float3(1, 1, 0),
	float3(1, -1, 0)
};

[domain("quad")]
DS_OUTPUT main(QuadTessParam input,
	float2 UV : SV_DomainLocation)
{
    DS_OUTPUT output;
    
    // Calculate the position of the current vertex within the quad
    float3 position = lerp(lerp(QuadPos[0], QuadPos[1], UV.y),
                           lerp(QuadPos[2], QuadPos[3], UV.y),
                           UV.x);
    
    // Extrude the vertex outward along its normal direction to create the spikes
    float3 normal = normalize(cross(ddx(position), ddy(position))); // calculate the normal
    float noise = noiseStrength * noise(position.xyz * 10.0f + time); // calculate the noise value
    position += normal * noise; // extrude the vertex outward along the normal direction
    
    output.pos = mul(mul(mul(float4(position, 1), model), view), projection); // transform the position to clip space
    output.texCoord = UV; // pass through the texture coordinates
    output.normal = normal; // pass through the normal
    
    return output;
}