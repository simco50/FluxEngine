#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"

cbuffer MaterialBuffer : register(b4)
{
	int cFrameCount;
}

#define cFrameCount 11

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
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
	int width, height;
	tDiffuseTexture.GetDimensions(width, height);

	float time = cElapsedTime;
	float duration = 1.0f;
	time = fmod(time, duration) / duration;

	int frameIndex = floor(time * cFrameCount);

	int3 loadCoordinates = int3(input.texCoord2.x * width, frameIndex, 0);
	float3 positionOffset = tDiffuseTexture.Load(loadCoordinates).xyz;

#define VERTEXANIMATION_SMOOTH_LERP
#ifdef VERTEXANIMATION_SMOOTH_LERP
	float t = fmod(time, 1.0f / cFrameCount) * cFrameCount;
	int nextFrameIndex = (frameIndex + 1) % 11;
	int3 nextLoadCoordinates = int3(input.texCoord2.x * width, nextFrameIndex, 0);
	float3 nextPositionOffset = tDiffuseTexture.Load(nextLoadCoordinates).xyz;
	positionOffset = positionOffset + t * (nextPositionOffset - positionOffset);
#endif

	float3 normal = tNormalTexture.Load(loadCoordinates).xyz * 2 - 1;
	normal = normalize(normal);

	output.position = mul(float4(input.position - positionOffset, 1.0f), cWorldViewProj);
	output.worldPosition = mul(float4(input.position - positionOffset, 1.0f), cWorld);

	output.normal = normalize(mul(input.normal - normal, (float3x3)cWorld));
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

	output.a = 0.9f;

	return output;
}
#endif