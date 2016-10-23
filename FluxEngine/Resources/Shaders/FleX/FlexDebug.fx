float4x4 gViewProj : VIEWPROJECTION; 
float4x4 gViewInv : VIEWINV;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4 gColor = float4(0, 0.2, 0.8, 1.0);
float gScale = 1.0f;
float gSpecularStrength = 0.6f;
float gShininess = 6.0f;

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 wPos : WORLDPOS;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 wPos : COLOR0;
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
	output.wPos = input.pos * gScale + input.wPos;
	output.pos = mul(float4(output.wPos, 1), gViewProj);
	output.normal = normalize(input.normal);
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 color_rgb = gColor.rgb;

	float3 viewDirection = normalize(input.wPos.xyz - gViewInv[3].xyz);

	float3 hv = -normalize(gLightDirection + viewDirection);
	float specularStrength = dot(input.normal, hv);
	specularStrength = gSpecularStrength * pow(saturate(specularStrength), gShininess);
	
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	float3 finalColor = color_rgb + specularStrength;

	return float4( finalColor , gColor.a );
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
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}