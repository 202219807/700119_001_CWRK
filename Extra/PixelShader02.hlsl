struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};


float noise(float2 p)
{
    return frac(sin(dot(p, float2(10.1898, 48.463))) * 32768.1473);
}

float fractalNoise(float2 p, int octaves, float persistence)
{
    float sum = 0.0f;
    float freq = 1.0f;
    float amp = 1.0f;

    for (int i = 0; i < octaves; i++)
    {
        sum += noise(p * freq) * amp;
        freq *= 2.0f;
        amp *= persistence;
    }

    return sum;
}

float4 main(PixelShaderInput input) : SV_TARGET
{   
    // Calculate a noise value based on the pixel coordinates
    float noiseVal = fractalNoise(input.pos.xy * 2.0f, 8, 0.1);

    // Use the noise value to create a color
    float4 objectColor = float4(noiseVal, 1.0f - noiseVal, 0.0f, 1.0f);

    // Multiply the color by the input color to blend them together
    float4 finalColor = objectColor * float4(input.color, 1.0);
    
    return finalColor;
   
}