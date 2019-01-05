#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Constants.hlsl"

float GetSpecularBlinnPhong(float3 viewDirection, float3 normal, float3 lightVector, float shininess)
{
	float3 halfVector = normalize(lightVector + viewDirection);
	float specularStrength = dot(normal, halfVector);
	return pow(saturate(specularStrength), shininess);
}

float GetSpecularPhong(float3 viewDirection, float3 normal, float3 lightVector, float shininess)
{
	float3 reflectedLight = reflect(-lightVector, normal);
	float specularStrength = dot(reflectedLight, -viewDirection);
	return pow(saturate(specularStrength), shininess);
}

float GetFresnelFalloff(float3 normal, float3 viewDirection, float fresnelPower, float fresnelMultiplier, float fresnelHardness)
{
	float fresnel = pow(1 - saturate(abs(dot(normal, viewDirection))), fresnelPower) * fresnelMultiplier;
	float fresnelMask = pow(1 - saturate(dot(float3(0, -1, 0), normal)), fresnelHardness);
	return fresnel * fresnelMask;
}

float4 CubeMapReflection(float3 normal, float3 viewDirection, float fresnelPower, float fresnelMultiplier, float fresnelHardness)
{
	float3 reflectV = reflect(viewDirection, normal);
	float4 reflectionSample = SampleCube(Cube, reflectV);
	return reflectionSample * GetFresnelFalloff(normal, viewDirection, 1.0f, 0.4f, 0.0f);
}

#ifdef NORMALMAP
float3 CalculateNormal(float3 normal, float3 tangent, float2 texCoord, bool invertY)
{
	float3 binormal = normalize(cross(tangent, normal));
	float3x3 normalMatrix = float3x3(tangent, binormal, normal);

#ifdef NORMALMAP_BC5
	float3 sampledNormal = float3(tNormalTexture.Sample(sNormalSampler, texCoord).rg, 0.0f);
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

struct LightResult
{
	float4 Diffuse;
	float4 Specular;
};

float4 DoDiffuse(Light light, float3 normal, float3 lightVector)
{
	return light.Color * max(dot(normal, lightVector), 0);
}

float4 DoSpecular(Light light, float3 normal, float3 lightVector, float3 viewDirection)
{
	return light.Color * GetSpecularPhong(viewDirection, normal, lightVector, 15.0f);
}

float DoAttenuation(Light light, float d)
{
    return 1.0f - smoothstep(light.Range * light.Attenuation, light.Range, d);
}

LightResult DoDirectionalLight(Light light, float3 normal, float3 viewDirection)
{
	LightResult result;
	result.Diffuse = light.Intensity * DoDiffuse(light, normal, -light.Direction);
	result.Specular = light.Intensity * DoSpecular(light, normal, -light.Direction, viewDirection);
	return result;
}

LightResult DoPointLight(Light light, float3 worldPosition, float3 normal, float3 viewDirection)
{
	LightResult result;
	float3 L = light.Position - worldPosition;
	float d = length(L);
	L = L / d;

	float attenuation = DoAttenuation(light, d);
	result.Diffuse = light.Intensity * attenuation * DoDiffuse(light, normal, L);
	result.Specular = light.Intensity * attenuation * DoSpecular(light, normal, L, viewDirection);
	return result;
}

LightResult DoSpotLight(Light light, float3 worldPosition, float3 normal, float3 viewDirection)
{
	LightResult result;

	float3 L = light.Position - worldPosition;
	float d = length(L);
	L = L / d;

	float minCos = cos(radians(light.SpotLightAngle));
	float maxCos = lerp(minCos, 1.0f, 1 - light.Attenuation);
	float cosAngle = dot(-L, light.Direction);
	float spotIntensity = smoothstep(minCos, maxCos, cosAngle);

	float attenuation = DoAttenuation(light, d);

	result.Diffuse = light.Intensity * attenuation * spotIntensity * DoDiffuse(light, normal, L);
	result.Specular = light.Intensity * attenuation * spotIntensity * DoSpecular(light, normal, L, viewDirection);
	return result;
}

LightResult DoLight(float3 worldPosition, float3 normal, float3 viewDirection)
{
	LightResult totalResult = (LightResult)0;

	for(uint i = 0; i < LIGHT_COUNT; ++i)
	{
		if(cLights[i].Enabled == 0)
		{
			continue;
		}

		if(cLights[i].Type != DIRECTIONAL_LIGHT && distance(worldPosition, cLights[i].Position) > cLights[i].Range)
		{
			continue;
		}

		LightResult result;

		switch(cLights[i].Type)
		{
		case DIRECTIONAL_LIGHT:
		{
			result = DoDirectionalLight(cLights[i], normal, viewDirection);
		}
		break;
		case POINT_LIGHT:
		{
			result = DoPointLight(cLights[i], worldPosition, normal, viewDirection);
		}
		break;
		case SPOT_LIGHT:
		{
			result = DoSpotLight(cLights[i], worldPosition, normal, viewDirection);
		}
		break;
		}

		totalResult.Diffuse += result.Diffuse;
		totalResult.Specular += result.Specular;
	}
	return totalResult;
}