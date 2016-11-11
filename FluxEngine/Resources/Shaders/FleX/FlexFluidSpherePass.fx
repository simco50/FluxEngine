float4x4 gViewProj : VIEWPROJECTION; 
float gScale = 1.0f;

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 wPos : WORLDPOS;
};
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 wPos : W_POSITION;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = BACK;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.wPos = input.pos * gScale + input.wPos.xyz;
	output.pos = mul(float4(output.wPos, 1), gViewProj);
	output.normal = normalize(input.normal);
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS(VS_OUTPUT input)
{
}


//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( NULL );
    }
}