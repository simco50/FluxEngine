#include "Uniforms.hlsl"
#include "Samplers.hlsl"

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
	float3 scale = 15.0f / 16.0f;
  	float3 offset = 1.0f / 32.0f;

	float3 sample = tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord).rgb;
	return tVolumeTexture.Sample(sVolumeSampler, scale * sample + offset);
}
