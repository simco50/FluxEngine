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

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 worldPosition : TEXCOORD1;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;

#ifdef NORMALMAP
	float3 tangent : TANGENT;
#endif
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

#ifdef NORMALMAP
	output.tangent = input.tangent;
#endif

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
	output.worldPosition = mul(transformedPosition, cWorld);
	output.normal = normalize(mul(transformedNormal, (float3x3)cWorld));

#else

	output.position = mul(float4(input.position, 1.0f), cWorldViewProj);
	output.worldPosition = mul(float4(input.position, 1.0f), cWorld);
	output.normal = normalize(mul(input.normal, (float3x3)cWorld));

#endif

	output.texCoord = input.texCoord;
	return output;
}
#endif

#ifdef COMPILE_PS

float4 PSMain(PS_INPUT input) : SV_TARGET
{
	float4 output = (float4)0;

	float3 normal = normalize(input.normal);

#ifdef NORMALMAP
	normal = CalculateNormal(normal, normalize(input.tangent), input.texCoord, false);
#endif

	float3 viewDirection = normalize(input.worldPosition.xyz - cViewInverse[3].xyz);
	LightResult result = DoLight(input.worldPosition, normal, viewDirection);
	float4 diffuse = result.Diffuse;
	float4 specular = result.Specular;

#ifdef DIFFUSEMAP
	diffuse *= Sample2D(Diffuse, input.texCoord);
#endif

#ifdef SPECULARMAP
	specular *= Sample2D(Specular, input.texCoord);
#endif

	output += diffuse;
	output += specular;

#ifdef ENVMAP
	output += CubeMapReflection(normal, viewDirection, 1.0f, 0.4f, 0.0f);
#endif

	return output;
}
#endif