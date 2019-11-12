#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"
//#include "SH.hlsl"
//#include "SphericalHarmonics.hlsl"

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
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 worldPosition : TEXCOORD1;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.position = mul(float4(input.position, 1.0f), cWorldViewProj);
	output.worldPosition = mul(float4(input.position, 1.0f), cWorld);
	output.normal = normalize(mul(input.normal, (float3x3)cWorld));
	output.texCoord = input.texCoord;
	return output;
}
#endif

#ifdef COMPILE_PS

float3 calcIrradiance(float3 nor, float4 sh[9]) {
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;

    return (
        c1 * sh[8] * (nor.x * nor.x - nor.y * nor.y) +
        c3 * sh[6] * nor.z * nor.z +
        c4 * sh[0] -
        c5 * sh[6] +
        2.0 * c1 * sh[4] * nor.x * nor.y -
        2.0 * c1 * sh[7]  * nor.x * nor.z +
        2.0 * c1 * sh[5] * nor.y * nor.z +
        2.0 * c2 * sh[3]  * nor.x +
        2.0 * c2 * sh[1] * nor.y -
        2.0 * c2 * sh[2]  * nor.z
    );
}

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

	float4 diffuse = float4(GetSHDiffuse(normal), 1);
	float4 specular = GetSpecularPhong(viewDirection, normal,-MainLightDir, 15.0f) * MainLightColor;
	return diffuse;
}
#endif