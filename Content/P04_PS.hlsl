#include "MathUtils.hlsli"

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // Lighting
    float shininess = 20.0;
    float3 K_a = float3(1.0, 1.0, 1.0);
    float3 K_d = float3(0.2, 0.2, 0.2);
    float3 K_s = float3(1.0, 1.0, 1.0);

    float3 lightPos = float3(10.0, 30.5, -20.0);
    float3 lightIntensity = float3(1.0, 1.0, 1.0);
    
	// Ambient light
    float3 ambientLight = K_a * input.normal;
	
    // Diffuse light
    float3 lightDir = normalize(lightPos - input.pos.xyz);
    float diffuse = max(dot(input.normal, lightDir), 0.0);
    float3 diffuseColor = lightIntensity * K_d * diffuse * input.normal;

    // Specular light
    float3 viewDir = normalize(-input.pos.xyz);
    float3 reflectDir = reflect(-lightDir, input.normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float3 specularColor = lightIntensity * K_s * specular;

    float4 finalColor = float4(ambientLight + diffuseColor + specularColor, 1.0);
    return finalColor;	
}
