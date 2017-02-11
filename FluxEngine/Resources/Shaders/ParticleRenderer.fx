float4x4 gWorld : WORLD;
float4x4 gViewProj : VIEWPROJ;
float4x4 gViewInverse : VIEWINV;
Texture2D gParticleTexture;

float PI = 3.141592;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

DepthStencilState DisableDepthWriting
{
	DepthEnable = TRUE;
	DepthFunc = ALWAYS;
};

BlendState AdditiveBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
};

BlendState AlphaBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
};

RasterizerState BackCulling
{
	CullMode = BACK;
};

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

VS_DATA MainVS(VS_DATA input)
{
	return input;
}

void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 origin, float3 offset, float2 texCoord, float4 col, float2x2 rotation)
{
	GS_DATA data = (GS_DATA)0;
	offset = float3(mul(offset.xy, rotation), offset.z);
	offset = mul(offset, (float3x3)gViewInverse);
	float3 pos = offset + origin;
	data.Position = mul(float4(pos, 1.0f), gViewProj);
	data.TexCoord = texCoord;
	data.Color = col;
	triStream.Append(data);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	float3 origin = mul(float4(vertex[0].Position, 1), gWorld).xyz;

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

float4 MainPS(GS_DATA input) : SV_TARGET 
{	
	float4 result = gParticleTexture.Sample(samPoint,input.TexCoord);
	return input.Color * result;
}

technique10 AlphaBlendingTechnique 
{
	pass p0 
	{
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetBlendState(AlphaBlending,float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
		SetRasterizerState(BackCulling);       
		SetDepthStencilState(DisableDepthWriting, 0);
	}
}

technique10 AdditiveBlendingTechnique 
{
	pass p0 
	{
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetBlendState(AdditiveBlending,float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
		SetRasterizerState(BackCulling);       
		SetDepthStencilState(DisableDepthWriting, 0);
	}
}