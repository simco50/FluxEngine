#include "Uniforms.hlsl"
#include "Samplers.hlsl"

cbuffer extraBuffer : register(b3)
{
	float cAberrationAmount;
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
	float4 sample1 = tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord - float2(cAberrationAmount, 0));
	float4 sample2 = tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord + float2(cAberrationAmount, 0));
	return float4(sample1.r, sample2.g, sample2.b, sample1.a);
}