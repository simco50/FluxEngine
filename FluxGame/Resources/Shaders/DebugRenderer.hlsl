#include "Uniforms.hlsl"
#include "Samplers.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.position = mul(float4(input.position, 1.0f), cViewProj);
	output.color = input.color;

	return output;
}
#endif

#ifdef COMPILE_PS

float4 PSMain(PS_INPUT input) : SV_TARGET
{
	return input.color;
}
#endif