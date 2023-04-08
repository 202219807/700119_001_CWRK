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

struct GS_INPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

struct GS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
};

// Transformations
float4 Transform(in float4 inPos)
{
    inPos.xyz *= 1.5;
    inPos.z -= 55.0;
    
    inPos.x -= smoothstep(0, 2.0, cos(inPos.yz)) * sin(time) * 3.0;
    inPos.y -= smoothstep(0, 1.0, cos(inPos.z)) * sin(time) * 0.8;
    // inPos.z -= smoothstep(0, 1.0, cos(inPos.xy)) * sin(time) * 1.8;

    // Create the rotation matrix
    float c = cos(time * 0.2);
    float s = sin(time * 0.2);
    float4x4 rotationMatrix = float4x4(
        c, 0, -s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1
    );
    inPos = mul(inPos, rotationMatrix);
    return inPos;
}

[maxvertexcount(6)]
void main(point GS_INPUT input[1], inout
	TriangleStream<GS_OUTPUT> OutputStream)
{
    GS_OUTPUT output = (GS_OUTPUT) 0;

    float4 vPos = input[0].pos;

    static const float3 g_positions[4] =
    {
        float3(-1, 1, 0),
		float3(-1, -1, 0),
		float3(1, 1, 0),
		float3(1, -1, 0),
    };

	// triangle 1 

    float quadSize = 0.15;

	// vertex 1:  
    output.pos = vPos + float4(quadSize * g_positions[1], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

	// vertex 2:  
    output.pos = vPos + float4(quadSize * g_positions[0], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

	// vertex 3:  
    output.pos = vPos + float4(quadSize * g_positions[2], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

    OutputStream.RestartStrip();

	//triangle 2 

    quadSize = 0.1;

	// vertex 1:  
    output.pos = vPos + float4(quadSize * g_positions[2], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

	// vertex 2:  
    output.pos = vPos + float4(quadSize * g_positions[3], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

	// vertex 3:  
    output.pos = vPos + float4(quadSize * g_positions[1], 0.0);
    output.pos = Transform(output.pos);
    //output.pos = mul(output.pos, model);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input[0].color;
    output.uv = (sign(input[0].pos.xy) + 1.0) / 2.0;
    OutputStream.Append(output);

    OutputStream.RestartStrip();
}