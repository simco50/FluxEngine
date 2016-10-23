float4x4 gViewProj : VIEWPROJECTION; 
float4x4 gViewInv : VIEWINV;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4 gColor = float4(0.0f, 0.8, 0.2f, 1.0);
float gSpecularStrength = 1.0f;
float gShininess = 50.0f;

bool gUseDiffuseTexture = false;
texture2D gTexture;

float3 gRigidRestposes[200];
float3 gRigidTranslations[200];
float4 gRigidRotations[200];

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 weight : WEIGHT;
	int4 cluster : CLUSTER;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 wPos : COLOR0;
	float2 texCoord : TEXCOORD;
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

float3 rotate(float4 q, float3 v)
{
	float3 a = cross(q.xyz, v) + q.w * v;
	return (cross(a, -q.xyz) + dot(q.xyz, v) * q.xyz + q.w * a);
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	float3 softNormal = float3(0,0,0);
	float3 softPos = float3(0,0,0);
	for(int w = 0; w < 4; ++w)
	{
		if(input.cluster[w] > -1)
		{
			int rigidIndex = input.cluster[w];
			float3 localPos = input.pos - gRigidRestposes[input.cluster[w]];
		
			float3 skinnedPos = gRigidTranslations[rigidIndex] + rotate(gRigidRotations[rigidIndex], localPos);
			float3 skinnedNormal = rotate(gRigidRotations[rigidIndex], normalize(input.normal));
			softPos += skinnedPos * input.weight[w];
			softNormal += skinnedNormal * input.weight[w];
		}
	}
	output.wPos = softPos;
	output.pos = mul(float4(output.wPos, 1), gViewProj);
	output.normal = normalize(softNormal);
	output.texCoord = input.texCoord;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 color_rgb = gColor.rgb;
	if (gUseDiffuseTexture)
	{
		color_rgb = gTexture.Sample(samLinear, input.texCoord).rgb;
	}

	float3 viewDirection = normalize(input.wPos.xyz - gViewInv[3].xyz);

	float3 hv = -normalize(gLightDirection + viewDirection);
	float specularStrength = dot(input.normal, hv);
	specularStrength = gSpecularStrength * pow(saturate(specularStrength), gShininess);
	
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	float3 finalColor = color_rgb + specularStrength;

	return float4( finalColor, gColor.a );
}

technique11 Default
{
    pass P0
    {
		SetRasterizerState(RS);
		SetDepthStencilState(DSS, 0);

        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}