#ifdef COMPILE_VS

cbuffer cPerFrameVS : register(b0)
{
	float cDeltaTimeVS;
	float cElapsedTimeVS;
}

cbuffer cPerViewVS : register(b1)
{
	float4x4 cViewVS;
	float4x4 cViewInverseVS;
	float4x4 cViewProjVS;
	float4 cFarClip;
	float4 cNearClip;
}

cbuffer cPerObjectVS : register(b2)
{
	float4x4 cWorldVS;
	float4x4 cWorldViewProjVS;
}

#endif

#ifdef COMPILE_GS

cbuffer cPerViewGS : register(b1)
{
	float4x4 cViewProjGS;
	float4x4 cViewInverseGS;
}

cbuffer cPerObjectGS : register(b2)
{
	float4x4 cWorldGS;
}

#endif

#ifdef COMPILE_PS

cbuffer cPerFramePS : register(b0)
{
	float cDeltaTimePS;
	float4 cLightDirectionPS;
}

cbuffer cPerViewPS : register(b1)
{
	float4x4 cViewPS;
	float4x4 cViewInversePS;
	float4x4 cViewProjectionPS;
}

cbuffer cPerObject : register(b2)
{
	float4x4 cWorldPS;
}

#endif