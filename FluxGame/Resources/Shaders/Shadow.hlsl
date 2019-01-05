#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
#ifdef NORMALMAP
	float3 tangent : TANGENT;
#endif

#ifdef SKINNED
	int4 boneIndex : BLENDINDEX;
	float4 vertexWeight : BLENDWEIGHT;
#endif
};

#ifdef COMPILE_VS
float4 VSMain(VS_INPUT input) : SV_POSITION
{
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

	return mul(transformedPosition, cWorldViewProj);
	
#else

	return mul(float4(input.position, 1.0f), cWorldViewProj);

#endif
}
#endif

#ifdef COMPILE_PS

void PSMain(float4 position : SV_POSITION)
{
	
}
#endif