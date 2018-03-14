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

VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)output;

	output.position.x = (float)(input.vertexId / 2) * 4.0f - 1.0f;
	output.position.y = (float)(input.vertexId % 2) * 4.0f - 1.0f;
	output.position.z = 0.0f;
	output.position.w = 1.0f;

	output.texCoord.x = (float)(input.vertexId / 2) * 2.0f;
	output.texCoord.y = 1.0f - (float)(input.vertexId % 2) * 2.0f;

	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
	float3 scale = 15.0f / 16.0f;
  	float3 offset = 1.0f / 32.0f;

	float3 sample = tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord).rgb;
	return tVolumeTexture.Sample(sVolumeSampler, scale * sample + offset);
}
