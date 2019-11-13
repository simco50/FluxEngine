#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"

cbuffer SHData : register(b0)
{
	float4 SHValues[7];
	float4 MainLightColor;
	float3 MainLightDir;
}

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
#ifdef NORMALMAP
	float3 tangent : TANGENT;
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
	output.position = mul(float4(input.position, 1.0f), cWorldViewProj);
	output.worldPosition = mul(float4(input.position, 1.0f), cWorld);
	output.normal = normalize(mul(input.normal, (float3x3)cWorld));

#ifdef NORMALMAP
	output.tangent = input.tangent;
#endif

	output.texCoord = input.texCoord;
	return output;
}
#endif

#ifdef COMPILE_PS

float3 GetSHDiffuse(float3 Normal)
{
	float4 NormalVector = float4(-Normal, 1);

	float3 Intermediate0, Intermediate1, Intermediate2;
	Intermediate0.x = dot(SHValues[0], NormalVector);
	Intermediate0.y = dot(SHValues[1], NormalVector);
	Intermediate0.z = dot(SHValues[2], NormalVector);

	float4 vB = NormalVector.xyzz * NormalVector.yzzx;
	Intermediate1.x = dot(SHValues[3], vB);
	Intermediate1.y = dot(SHValues[4], vB);
	Intermediate1.z = dot(SHValues[5], vB);

	float vC = NormalVector.x * NormalVector.x - NormalVector.y * NormalVector.y;
	Intermediate2 = SHValues[6].xyz * vC;

	// max to not get negative colors
	return max(0, Intermediate0 + Intermediate1 + Intermediate2);
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
	float4 output = (float4)0;
	float3 normal = normalize(input.normal);
	float3 viewDirection = normalize(input.worldPosition.xyz - cViewInverse[3].xyz);

#ifdef NORMALMAP
	normal = CalculateNormal(normal, normalize(input.tangent), input.texCoord, false);
#endif

	float4 diffuse = float4(GetSHDiffuse(normal), 1);
	float4 specular = GetSpecularPhong(viewDirection, normal,-MainLightDir, 15.0f) * MainLightColor;
	return diffuse;
}
#endif