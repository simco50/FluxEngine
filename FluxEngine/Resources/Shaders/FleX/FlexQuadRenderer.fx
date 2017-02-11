float3 gLightDirection = float3(0, 0, 1);
float gScale = 1.0f;
float4x4 gViewProj : VIEWPROJ;
float4x4 gViewInv : VIEWINV;

float4x4 gView : VIEW;
float4x4 gProj : PROJ;

struct VS_DATA
{
	float3 pos : POSITION;
};

struct GS_DATA
{
	float4 pos : SV_POSITION;
	float3 wPos : WORLDPOS;
	float2 texCoord: TEXCOORD0;
};

DepthStencilState DSS
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

RasterizerState RS
{
	CullMode = BACK;
};

VS_DATA Main_VS(VS_DATA input)
{
	return input;
}

void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float3 offset, float2 texCoord)
{
	GS_DATA data = (GS_DATA)0;
	data.wPos = mul(float4(pos, 1), gView);
	float3 cornerPos = pos + mul(offset, (float3x3)gViewInv);
	data.pos = mul(float4(cornerPos, 1.0f), gViewProj);
	data.texCoord = texCoord;

	triStream.Append(data);
}

[maxvertexcount(4)]
void Main_GS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	float3 topLeft, topRight, bottomLeft, bottomRight;

	topLeft = float3(-gScale / 2.0f, gScale / 2.0f, 0.0f);
	topRight = float3(gScale / 2.0f, gScale / 2.0f, 0.0f);
	bottomLeft = float3(-gScale / 2.0f, -gScale / 2.0f, 0.0f);
	bottomRight = float3(gScale / 2.0f, -gScale / 2.0f, 0.0f);

	CreateVertex(triStream, vertex[0].pos, bottomLeft, float2(0, 1));
	CreateVertex(triStream, vertex[0].pos, topLeft, float2(0, 0));
	CreateVertex(triStream, vertex[0].pos, bottomRight, float2(1, 1));
	CreateVertex(triStream, vertex[0].pos, topRight, float2(1, 0));
}

float4 Main_PS(GS_DATA input) : SV_TARGET
{
	//float de = input.pos.z;
	//return float4(de, 0, 0, 1);

	//Calculate the view space normal
	float3 normal;
	normal.xy = input.texCoord * 2.0f - 1.0f;
	float r2 = dot(normal.xy, normal.xy);
	//Clip if the pixel falls out the sphere
	clip(r2 > 1.0f ? -1 : 1);
	normal.z = -sqrt(1.0f - r2);
	normal = normalize(normal);

	//calculate the depth
	float4 pixelPos = float4(input.wPos + normal * gScale, 1.0f);
	float4 clipPos = mul(pixelPos, gProj);
	float d = (clipPos.z - 1.0f) / (100.0f - 1.0f);
	return float4(d, 0, 0, 1);
}

technique11 Default
{
	pass P0
	{
		SetRasterizerState(RS);
		SetDepthStencilState(DSS, 0);

		SetVertexShader(CompileShader(vs_4_0, Main_VS()));
		SetGeometryShader(CompileShader(gs_4_0, Main_GS()));
		SetPixelShader(CompileShader(ps_4_0, Main_PS()));
	}
}