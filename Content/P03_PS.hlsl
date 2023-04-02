#include "MathUtils.hlsli"

struct PS_INPUT
{
	float4 pos		: SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal	: TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float3 lightColor = float3(1.0, 1.0, 1.0);
	
    float3 diffuse = saturate(dot(input.normal, lightDir)) * lightColor;
    float3 specular = 0.0;

    if (dot(input.normal, lightDir) > 0.0)
    {
        float3 viewDir = normalize(-input.pos.xyz);
        float3 halfVec = normalize(lightDir + viewDir);
        specular = pow(saturate(dot(halfVec, input.normal)), 10.0) * lightColor;
    }

    float noiseValue = perlin(input.texCoord * 5.0);
    float4 textureColor = float4(noiseValue, noiseValue, noiseValue, 1.0);
      
    float3 finalColor = textureColor.xyz * diffuse + specular;
    return float4(finalColor, 1.0);

}
