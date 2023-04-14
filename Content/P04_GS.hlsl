#include "MathUtils.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct GS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : TEXCOORD0;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : TEXCOORD0;
};

[maxvertexcount(24)]
void main(
	triangle GS_INPUT input[3] : SV_POSITION,
	inout TriangleStream<GS_OUTPUT> OutputStream
)
{
    // Create the triangles from the input points
    GS_OUTPUT output = (GS_OUTPUT) 0;

    float3 p0 = input[0].pos.xyz;
    float3 p1 = input[1].pos.xyz;
    float3 p2 = input[2].pos.xyz;

    float3 m0 = (p0 + p1) / 2.0;
    float3 m1 = (p1 + p2) / 2.0;
    float3 m2 = (p2 + p0) / 2.0;
    
    // Triangle 1
    
    // Calculate face normal
    float3 edge1 = m0 - p0;
    float3 edge2 = m2 - p0;
    float3 faceNormal = normalize(cross(edge1, edge2));
    
    output.pos = float4(p0 + float3(1.0, 1.0, 1.0) * faceNormal, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0); // float4(1.0, 0.0, 0.0, 1.0); // input[0].color;
    OutputStream.Append(output);
    
    output.pos = float4(m0, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    output.pos = float4(m2, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    // End the current primitive
    OutputStream.RestartStrip();
        
    // Triangle 2

    // Calculate face normal
    edge1 = p1 - m0;
    edge2 = m1 - m0;
    faceNormal = normalize(cross(edge1, edge2));
    
    output.pos = float4(m0, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    output.pos = float4(p1 + float3(1.0, 1.0, 1.0) * faceNormal, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    output.pos = float4(m1, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    // End the current primitive
    OutputStream.RestartStrip();
    
    
    // Triangle 3
    
    // Calculate face normal
    edge1 = m1 - m2;
    edge2 = p2 - m2;
    faceNormal = normalize(cross(edge1, edge2));
    
    output.pos = float4(m2, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    output.pos = float4(m1, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = -faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    output.pos = float4(p2 + float3(1.0, 1.0, 1.0) * faceNormal, 1.0);
    output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.normal = faceNormal;
    output.color = float4((sign(p0.xy) + 1.0) / 2.0, 1.0, 1.0);
    OutputStream.Append(output);
    
    // End the current primitive
    OutputStream.RestartStrip();
}
