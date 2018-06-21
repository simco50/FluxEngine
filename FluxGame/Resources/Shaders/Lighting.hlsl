#include "Samplers.hlsl"

#ifdef SPECULARMAP
float GetSpecularBlinnPhong(float3 viewDirection, float3 normal, float2 texCoord, float shininess)
{
	float3 halfVector = -normalize(cLightDirection + viewDirection);
	float specularStrength = dot(normal, halfVector);
	float specularSample = tSpecularTexture.Sample(sSpecularSampler, texCoord).r;
	return saturate(pow(specularStrength, shininess) * specularSample);
}

float GetSpecularPhong(float3 viewDirection, float3 normal, float2 texCoord, float shininess)
{
	float3 reflectedLight = reflect(cLightDirection, normal);
	float specularStrength = dot(reflectedLight, -viewDirection);
	float specularSample = tSpecularTexture.Sample(sSpecularSampler, texCoord).r;
	return saturate(pow(specularStrength, shininess) * specularSample);
}
#endif

float GetFresnelFalloff(float3 normal, float3 viewDirection, float fresnelPower, float fresnelMultiplier, float fresnelHardness)
{
	float fresnel = pow(1 - saturate(abs(dot(normal, viewDirection))), fresnelPower) * fresnelMultiplier;
	float fresnelMask = pow(1 - saturate(dot(float3(0, -1, 0), normal)), fresnelHardness);
	return fresnel * fresnelMask;
}

#ifdef NORMALMAP
float3 CalculateNormal(float3 normal, float3 tangent, float2 texCoord, bool invertY)
{
	float3 binormal = normalize(cross(tangent, normal));
	float3x3 normalMatrix = float3x3(tangent, binormal, normal);

#ifdef NORMALMAP_BC5
	float3 sampledNormal = float3( tNormalTexture.Sample(sNormalSampler, texCoord).rg, 0.0f);
	sampledNormal.xy = sampledNormal.xy * 2.0f - 1.0f;
	sampledNormal.z = sqrt(1 - dot(sampledNormal.xy, sampledNormal.xy));
#else
	float3 sampledNormal = tNormalTexture.Sample(sNormalSampler, texCoord).rgb;
	sampledNormal = sampledNormal * 2.0f - 1.0f;
	if(invertY)
		sampledNormal.y = -sampledNormal.y;
	sampledNormal = saturate(sampledNormal);
#endif

	return mul(sampledNormal, normalMatrix);
}
#endif