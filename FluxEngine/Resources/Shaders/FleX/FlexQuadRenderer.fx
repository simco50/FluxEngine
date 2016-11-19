float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gScale = 1.0f;
float4x4 gViewProj : VIEWPROJ;
float4x4 gViewInv : VIEWINV;

struct VS_DATA
{
	float3 pos : POSITION;
};

struct GS_DATA
{
	float4 pos : SV_POSITION;
	float3 eyeSpacePos : WORLDPOS;
	float2 texCoord: TEXCOORD0;
	float3x3 rotation : ROTATION;
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

void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float2 texCoord, float3x3 rotation)
{
	GS_DATA data = (GS_DATA)0;
	data.pos = mul(float4(pos, 1.0f), gViewProj);
	data.texCoord = texCoord;
	data.eyeSpacePos = pos;
	data.rotation = rotation;

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

	topLeft = mul(topLeft, (float3x3)gViewInv);
	topRight = mul(topRight, (float3x3)gViewInv);
	bottomLeft = mul(bottomLeft, (float3x3)gViewInv);
	bottomRight = mul(bottomRight, (float3x3)gViewInv);

	float3 right = normalize(topRight - topLeft);
	float3 up = normalize(topRight - bottomRight);
	float3 fwd = normalize(cross(up, right));
	float3x3 rotation = transpose(float3x3(right, up, fwd));

	topLeft += vertex[0].pos;
	topRight += vertex[0].pos;
	bottomLeft += vertex[0].pos;
	bottomRight += vertex[0].pos;

	CreateVertex(triStream, bottomLeft, float2(0, 1), rotation);
	CreateVertex(triStream, topLeft, float2(0, 0), rotation);
	CreateVertex(triStream, bottomRight, float2(1, 1), rotation);
	CreateVertex(triStream, topRight, float2(1, 0), rotation);
}

float4 Main_PS(GS_DATA input) : SV_TARGET
{


	//Calculate the normal
	float3 normal;
	normal.xy = input.texCoord * 2.0f - 1.0f;
	float r2 = dot(normal.xy, normal.xy);
	//Clip if the pixel falls out the sphere
	clip(r2 > 1.0f ? -1 : 1);
	normal.z = sqrt(1.0f - r2);

	//calculate the depth
	float3 pixelPos = float3(input.eyeSpacePos + normalize(normal) * gScale);
	float4 clipSpacePos = mul(float4(pixelPos, 1.0f), gViewProj);
	float d = saturate(1.0f - (clipSpacePos.z / 200.0f));
	return float4(d, 0, 0, 0);
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