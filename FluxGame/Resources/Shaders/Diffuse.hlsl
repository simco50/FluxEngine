#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Constants.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;

#ifdef SKINNED
	int4 boneIndex : BLENDINDEX;
	float4 vertexWeight : BLENDWEIGHT;
#endif
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

#ifdef SKINNED

	float4x4 finalMatrix;
	for(int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
	{
		if(input.boneIndex[i] == -1)
		{
			break;
		}
		finalMatrix += input.vertexWeight[i] * cSkinMatrices[input.boneIndex[i]];
	}

	float4 transformedPosition = mul(float4(input.position, 1.0f), finalMatrix);
	float3 transformedNormal = mul(input.normal, (float3x3)finalMatrix);

	output.position = mul(transformedPosition, cWorldViewProj);
	output.normal = normalize(mul(transformedNormal, (float3x3)cWorld));

#else

	output.position = mul(float4(input.position, 1.0f), cWorldViewProj);
	output.normal = normalize(mul(input.normal, (float3x3)cWorld));

#endif

	output.texCoord = input.texCoord;
	return output;
}
#endif

#ifdef COMPILE_PS

float4 PSMain(PS_INPUT input) : SV_TARGET
{
	float3 normal = normalize(input.normal);
	float diffuseStrength = saturate(dot(normal, -cLightDirection));

	float4 sample = tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord);
	return float4(sample.rgb * diffuseStrength, 1.0f);
}
#endif