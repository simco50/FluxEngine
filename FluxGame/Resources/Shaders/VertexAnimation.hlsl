#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"

cbuffer MaterialBuffer : register(b4)
{
	int cFrameCount;
	float cDuration;
}

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
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

float3 MaxToFlux(float3 input)
{
	float t = input.z;
	input.z = input.y;
	input.y = -t;
	input.x *= -1;
	return input;
}

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	int width, height;
	tPositionMorphingTexture.GetDimensions(width, height);

	float time = cElapsedTime;
	time = fmod(time, cDuration) / cDuration;

	int frameIndex = floor(time * cFrameCount);

	int3 loadCoordinates = int3(input.texCoord2.x * width, frameIndex, 0);
	float3 positionOffset = tPositionMorphingTexture.Load(loadCoordinates).xyz;

	float t = fmod(time, 1.0f / cFrameCount) * cFrameCount;
	int nextFrameIndex = (frameIndex + 1) % cFrameCount;
	int3 nextLoadCoordinates = int3(input.texCoord2.x * width, nextFrameIndex, 0);
	float3 nextPositionOffset = tPositionMorphingTexture.Load(nextLoadCoordinates).xyz;
	positionOffset = positionOffset + t * (nextPositionOffset - positionOffset);

	float3 normal = tNormalMorphingTexture.Load(loadCoordinates).xyz * 2 - 1;
	normal = normalize(normal);

	//positionOffset = MaxToFlux(positionOffset);
	//normal = MaxToFlux(normal);

	output.position = mul(float4(input.position - positionOffset, 1.0f), cWorldViewProj);
	output.worldPosition = mul(float4(input.position - positionOffset, 1.0f), cWorld);

	output.normal = normalize(mul(input.normal - normal, (float3x3)cWorld));

#ifdef NORMALMAP
	output.tangent = input.tangent;
#endif

	output.texCoord = input.texCoord;
	return output;
}
#endif