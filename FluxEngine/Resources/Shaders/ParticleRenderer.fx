float4x4 gWorld : WORLD;
float4x4 gViewProj : VIEWPROJ;
float4x4 gViewInverse : VIEWINV;
Texture2D gParticleTexture;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

DepthStencilState DisableDepthWriting
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
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

void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float2 texCoord, float4 col, float2x2 uvRotation)
{
	GS_DATA data = (GS_DATA)0;
	data.Position = mul(float4(pos, 1.0f), gViewProj);
	texCoord -= float2(0.5f,0.5f);
	texCoord = mul(texCoord, uvRotation);
	texCoord += float2(0.5f,0.5f);
	data.TexCoord = texCoord;
	data.Color = col;
	triStream.Append(data);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	float3 topLeft, topRight, bottomLeft, bottomRight;
	float size = vertex[0].Size;
	float3 origin = mul(float4(vertex[0].Position, 1), gWorld).xyz;

	topLeft = float3(-size / 2.0f, size / 2.0f, 0.0f);
	topRight = float3(size / 2.0f, size / 2.0f, 0.0f);
	bottomLeft = float3(-size / 2.0f, -size / 2.0f, 0.0f);
	bottomRight = float3(size / 2.0f, -size / 2.0f, 0.0f);

	topLeft = mul(topLeft, (float3x3)gViewInverse);
	topRight = mul(topRight, (float3x3)gViewInverse);
	bottomLeft = mul(bottomLeft, (float3x3)gViewInverse);
	bottomRight = mul(bottomRight, (float3x3)gViewInverse);

	topLeft += origin;
	topRight += origin;
	bottomLeft += origin;
	bottomRight += origin;

	float2x2 uvRotation = {cos(vertex[0].Rotation), - sin(vertex[0].Rotation), sin(vertex[0].Rotation), cos(vertex[0].Rotation)};

	CreateVertex(triStream,bottomLeft, float2(0,1), vertex[0].Color, uvRotation);
	CreateVertex(triStream,topLeft, float2(0,0), vertex[0].Color, uvRotation);
	CreateVertex(triStream,bottomRight, float2(1,1), vertex[0].Color, uvRotation);
	CreateVertex(triStream,topRight, float2(1,0), vertex[0].Color, uvRotation);
}

float4 MainPS(GS_DATA input) : SV_TARGET 
{	
	float4 result = gParticleTexture.Sample(samPoint,input.TexCoord);
	return input.Color * result;
}

technique10 Default 
{
	pass p0 
	{
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
		SetRasterizerState(BackCulling);       
		SetDepthStencilState(DisableDepthWriting, 0);
	}
}
