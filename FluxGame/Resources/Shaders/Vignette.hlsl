#include "Uniforms.hlsl"
#include "Samplers.hlsl"

cbuffer MaterialBuffer : register(b3)
{
	float cRadius;
	float cSmoothness;
}

struct VS_INPUT
{
	uint vertexId : SV_VertexID;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float center = length(input.texCoord - float2(0.5f, 0.5f));
	float vignette = smoothstep(cRadius, cRadius - cSmoothness, center);
	float4 sample = Sample2D(Diffuse, input.texCoord);
	return float4(sample.rgb * vignette, sample.a);
}