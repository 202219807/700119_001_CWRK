cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    float4 timer;
    float4 resolution;
    float4 eye;
};

struct VertexShaderInput
{
    float3 pos      : POSITION;
    float3 color    : COLOR0;
};

struct GeometryShaderInput
{
    float4 pos      : SV_POSITION;
    float3 color    : COLOR0;
};

static float time = timer.x;

float3 GradientShade(float3 position)
{
    // Define the start and end colors of the gradient
    float3 startColor = float3(0.0, 1.0, 1.0); // Blue
    float3 endColor = float3(1.0, 0.0, 0.0);   // Yellow

    // Calculate the position's distance from the origin
    float distance = length(position);

    // Normalize the position vector to get a value between 0 and 1
    float t = saturate(dot(normalize(position), normalize(float3(-1.0, 1.0, 0.0))));

    // Add variation to the colors
    float3 colorVariationVector = sin(position * 0.6);
    startColor += colorVariationVector;
    endColor += colorVariationVector;

    // Interpolate between the start and end colors based on the distance
    return lerp(startColor, endColor, t);
}

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;

    float4 inPos = float4(input.pos, 1.0);

    // Transformations
    inPos.xyz *= 0.5;

    inPos.z -= 15.0;
    inPos.y += smoothstep(0, 1, cos(inPos.y)) * sin(time);

    // Create the rotation matrix
    float c = cos(time * 0.1);
    float s = sin(time * 0.1);
    float4x4 rotationMatrix = float4x4(
        c, 0, -s, 0,
        0, 1, 0, 0,
        s, 0, c, 0,
        0, 0, 0, 1
        );

    inPos = mul(inPos, rotationMatrix);

    output.pos = inPos;
    output.color = GradientShade(output.pos);

    return output;
}