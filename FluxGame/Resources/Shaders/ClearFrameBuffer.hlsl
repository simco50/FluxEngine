#include "Uniforms.hlsl"
#include "Samplers.hlsl"

cbuffer cCustom : register(b4)
{
	float4 cColor;
}

struct VS_INPUT
{
	float3 position : POSITION;
};

struct PS_OUTPUT
{
	float4 position : SV_POSITION;
};

void VSMain(VS_INPUT input, out PS_OUTPUT output)
{
	output.position = mul(float4(input.position, 1.0f), cWorld);
}

void PSMain(PS_OUTPUT input, out float4 output : SV_TARGET)
{
	output = cColor;
}
