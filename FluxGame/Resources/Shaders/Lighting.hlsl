#include "Samplers.hlsl"

#ifdef SPECULARMAP
float GetSpecularBlinnPhong(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 halfVector = -normalize(cLightDirection + viewDirection);
	float specularStrength = dot(normal, halfVector);
	float specularSample = tSpecularTexture.Sample(sSpecularSampler, texCoord).r;
	return saturate(pow(specularStrength, 5) * specularSample);
}

float GetSpecularPhong(float3 viewDirection, float3 normal, float2 texCoord)
{
	float3 reflectedLight = reflect(-cLightDirection, normal);
	float specularStrength = dot(reflectedLight, -viewDirection);
	float specularSample = tSpecularTexture.Sample(sSpecularSampler, texCoord).r;
	return saturate(pow(specularStrength, 5) * specularSample);
}
#endif

#ifdef NORMALMAP
float3 CalculateNormal(float3 normal, float3 tangent, float2 texCoord, bool invertY)
{
	float3 binormal = normalize(cross(tangent, normal));
	float3x3 normalMatrix = float3x3(tangent, binormal, normal);
	float3 sampledNormal = tNormalTexture.Sample(sNormalSampler, texCoord);
	sampledNormal = sampledNormal * 2.0f - 1.0f;
	if(invertY)
		sampledNormal.y = -sampledNormal.y;
	sampledNormal = saturate(sampledNormal);
	return mul(sampledNormal, normalMatrix);
}
#endif