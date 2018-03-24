#include "Uniforms.hlsl"
#include "Samplers.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;

#ifdef SKINNED
	int4 boneIndex : BLENDINDICES
	float4 vertexWeight : BLENDWEIGHTS;
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
	float4 originalPosition = float4(input.position, 1);
	float4 transformedPosition = 0;
	float3 transformedNormal = 0;

	for(int i = 0; i < 4; ++i)
	{
		int boneIndex = input.boneIndex[i];
		if(boneIndex > -1)
		{
			transformedPosition += input.vertexWeight[i] * mul(originalPosition, cSkinMatrices[boneIdx]);
			transformedNormal += input.vertexWeight[i] * mul(input.normal, (float3x3)cSkinMatrices[boneIdx]);
			transformedPosition.w = 1;
		}
	}
	output.position = mul(float4(transformedPosition, 1.0f), cWorldViewProj);
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