float PI = 3.141592;

#include "Uniforms.hlsl"
#include "Samplers.hlsl"

struct VS_DATA
{
	float3 Position : POSITION;
	float4 Color: COLOR;
	float Size: TEXCOORD0;
	float Rotation : TEXCOORD1;
};

struct GS_DATA
{
	float4 Position : SV_POSITION;
	float2 TexCoord: TEXCOORD0;
	float4 Color : COLOR;
};

#ifdef COMPILE_VS
VS_DATA VSMain(VS_DATA input)
{
	return input;
}
#endif

#ifdef COMPILE_GS
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 origin, float3 offset, float2 texCoord, float4 col, float2x2 rotation)
{
	GS_DATA data = (GS_DATA)0;
	offset = float3(mul(offset.xy, rotation), offset.z);
	offset = mul(offset, (float3x3)cViewInverseGS);
	float3 pos = offset + origin;
	data.Position = mul(float4(pos, 1.0f), cViewProjGS);
	data.TexCoord = texCoord;
	data.Color = col;
	triStream.Append(data);
}

[maxvertexcount(4)]
void GSMain(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	float3 origin = mul(float4(vertex[0].Position, 1), cWorldGS).xyz;

	float3 topLeft = float3(-vertex[0].Size / 2.0f, vertex[0].Size / 2.0f, 0.0f);
	float3 topRight = float3(vertex[0].Size / 2.0f, vertex[0].Size / 2.0f, 0.0f);
	float3 bottomLeft = float3(-vertex[0].Size / 2.0f, -vertex[0].Size / 2.0f, 0.0f);
	float3 bottomRight = float3(vertex[0].Size / 2.0f, -vertex[0].Size / 2.0f, 0.0f);

	float rad = vertex[0].Rotation * PI / 180.0f;
	float2x2 rotation = {cos(rad), -sin(rad), sin(rad), cos(rad)};

	CreateVertex(triStream, origin, bottomLeft, float2(0,1), vertex[0].Color, rotation);
	CreateVertex(triStream, origin, topLeft, float2(0,0), vertex[0].Color, rotation);
	CreateVertex(triStream, origin, bottomRight, float2(1,1), vertex[0].Color, rotation);
	CreateVertex(triStream, origin, topRight, float2(1,0), vertex[0].Color, rotation);
}
#endif

#ifdef COMPILE_PS
float4 PSMain(GS_DATA input) : SV_TARGET 
{	
	float4 result = tDiffuseTexture.Sample(sDiffuseSampler, input.TexCoord);
	return input.Color * result;
}
#endif