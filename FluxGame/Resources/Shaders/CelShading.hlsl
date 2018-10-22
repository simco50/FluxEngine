#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"
#ifdef SKINNED
#include "Skinning.hlsl"
#endif

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
    float edge : TEXCOORD0;
    float4 diffuse : TEXCOORD1;
    float4 specular : TEXCOORD2;
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

#ifdef SKINNED

#ifdef DUAL_QUATERNION
	float2x4 dualQuaternion = BlendBoneTransformsToDualQuaternion(input.boneIndex, input.vertexWeight);
	float4 transformedPosition = float4(DualQuatTransformPoint(input.position, dualQuaternion[0], dualQuaternion[1]), 1);
	float3 transformedNormal = QuaternionRotateVector(input.normal, dualQuaternion[0]);
#else
	float4x4 finalMatrix = BlendBoneTransformsToMatrix(input.boneIndex, input.vertexWeight);
	float4 transformedPosition = mul(float4(input.position, 1.0f), finalMatrix);
	float3 transformedNormal = mul(input.normal, (float3x3)finalMatrix);
#endif

	output.position = mul(transformedPosition, cWorldViewProj);
	float3 normal = normalize(mul(transformedNormal, (float3x3)cWorld));

#else
	output.position = mul(float4(input.position, 1.0f), cWorldViewProj);
	float3 normal = normalize(mul(input.normal, (float3x3)cWorld));
#endif
	float3 worldPosition = mul(transformedPosition, cWorld);
	float3 viewDirection = normalize(worldPosition.xyz - cViewInverse[3].xyz);
    LightResult result = DoLight(worldPosition, normal, viewDirection);
    output.specular = result.Specular;
    output.diffuse = result.Diffuse;
    output.edge = max(dot(normal, -viewDirection), 0);

	return output;
}
#endif

#ifdef COMPILE_PS

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    if(input.edge < 0.3f)
        return float4(1,1,1,1);
    float x = (float)((int)(input.diffuse.x * 10)) / 10;
    float y = (float)((int)(input.diffuse.y * 10)) / 10;
    float z = (float)((int)(input.diffuse.z * 10)) / 10;

	return float4(x,y,z,1) + input.specular;
}
#endif