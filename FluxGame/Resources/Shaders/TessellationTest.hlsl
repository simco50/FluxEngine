#include "Constants.hlsl"
#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Lighting.hlsl"
#include "Tessellation.hlsl"

cbuffer MaterialBuffer : register(b4)
{
	float cTessellation;
	float cDisplacement;
}

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
#ifdef NORMALMAP
	float3 tangent : TANGENT;
#endif
};

struct HS_CONSTANT_OUTPUT
{
	float edges[3] : SV_TESSFACTOR;
	float inside : SV_INSIDETESSFACTOR;
};

struct HS_OUTPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;

#ifdef NORMALMAP
	float3 tangent : TANGENT;
#endif
};

struct DS_OUTPUT
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
VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.position = input.position;
	output.texCoord = input.texCoord;
	output.normal = input.normal;
#ifdef NORMALMAP
	output.tangent = input.tangent;
#endif

	return output;
}
#endif

#ifdef COMPILE_HS

HS_CONSTANT_OUTPUT HSConst(InputPatch<VS_OUTPUT, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT output;

	float4 tess = EdgeLengthBasedTessellation(
		float4(inputPatch[0].position, 1), 
		float4(inputPatch[1].position, 1), 
		float4(inputPatch[2].position, 1), 
		50.0f / cTessellation
		);

    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = tess.x;
    output.edges[1] = tess.y;
    output.edges[2] = tess.z;
    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = tess.w;
    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConst")]
HS_OUTPUT HSMain(InputPatch<VS_OUTPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HS_OUTPUT output;
	output.position = patch[pointId].position;
	output.texCoord = patch[pointId].texCoord;
	output.normal = patch[pointId].normal;
#ifdef NORMALMAP
	output.tangent = patch[pointId].tangent;
#endif
	return output;
}

#endif

#ifdef COMPILE_DS

[domain("tri")]
DS_OUTPUT DSMain(HS_CONSTANT_OUTPUT input, OutputPatch<HS_OUTPUT, 3> patch, float3 uvwCoord : SV_DOMAINLOCATION)
{
	DS_OUTPUT output;
	float4 pos = float4(uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position, 1);
	float2 texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;
	float3 normal = normalize(uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal);
	float offset = tNormalTexture.SampleLevel(sNormalSampler, texCoord, 0.0f).r;
    pos = mul(pos, cWorld);
	pos -= float4(normal * offset * cDisplacement, 0);
	pos += float4(normal * cDisplacement, 0);
    output.position = mul(pos, cViewProj);
    output.worldPosition = pos;
	output.texCoord = texCoord;
	output.normal = mul(normal, (float3x3)cWorld);
	
#ifdef NORMALMAP
	float3 tangent = uvwCoord.x * patch[0].tangent + uvwCoord.y * patch[1].tangent + uvwCoord.z * patch[2].tangent;
	output.tangent = tangent;
#endif

	return output;
}

#endif

#ifdef COMPILE_PS

float4 PSMain(DS_OUTPUT input) : SV_TARGET
{
	float4 output = (float4)0;

	//float3 normal = normalize(input.normal);
	float3 normal = FindNormal(tNormalTexture, sNormalSampler, input.texCoord, 0.01f, 0.01f);

#ifdef NORMALMAP
	normal = CalculateNormal(normal, normalize(input.tangent), input.texCoord, false);
#endif

	float3 viewDirection = normalize(input.worldPosition.xyz - cViewInverse[3].xyz);
	LightResult result = DoLight(input.worldPosition, normal, viewDirection);
	float4 diffuse = result.Diffuse;

#ifdef DIFFUSEMAP
	float amount = Sample2D(Normal, input.texCoord).r;
	diffuse *= lerp(Sample2D(Diffuse, 5*input.texCoord), Sample2D(Specular, 5*input.texCoord), amount);
#endif

	output += diffuse;

	return output;
}
#endif