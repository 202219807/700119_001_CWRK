#define PI radians(180.)

/* Preprocessor directives for porting GLSL code to HLSL */
#define vec2 float2 
#define vec3 float3 
#define vec4 float4 
#define mat2 float2x2 
#define mat3 float3x3 
#define mat4 float4x4 

#define mod  fmod
#define mix  lerp
#define atan atan2

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 timer;
	float4 resolution;
};

static float time = timer.x;

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

mat4 persp(float fov, float aspect, float zNear, float zFar) {
	float f = tan(PI * 0.5 - 0.5 * fov);
	float rangeInv = 1.0 / (zNear - zFar);

	return mat4(
		f / aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (zNear + zFar) * rangeInv, -1,
		0, 0, zNear * zFar * rangeInv * 2., 0);
}

mat4 rotY(float angleInRadians) {
	float s = sin(angleInRadians);
	float c = cos(angleInRadians);

	return mat4(
		c, 0, -s, 0,
		0, 1, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1);
}

vec3 SampleSpherePos(float idx, float num) {
	idx += 0.5;
	float phi = 10.166407384630519631619018026484 * idx;
	float th_cs = 1.0 - 2.0 * idx / num;
	float th_sn = sqrt(clamp(1.0 - th_cs * th_cs, 0.0, 1.0));
	return vec3(cos(phi) * th_sn, sin(phi) * th_sn, th_cs);
}

PixelShaderInput main(VertexShaderInput input, float pSize: PSIZE, uint vertexId: SV_VertexID) {
	PixelShaderInput output;
	float vertexCount = 486;
	vec4 vertPos = mul(rotY(time * 0.1),vec4(SampleSpherePos(vertexId, vertexCount), 1.0) + vec4(0, 0, -3.0, 0));

	output.pos = mul(persp(PI * 0.25, resolution.x / resolution.y, 0.1, 100.0), vertPos);
	pSize = 5.0;

	output.color = vec4(1, 1, 1, 1);
	return output;
}