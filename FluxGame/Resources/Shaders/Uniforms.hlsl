cbuffer cPerFrame : register(b0)
{
	float cDeltaTime;
	float cElapsedTime;
}

cbuffer cPerView : register(b1)
{
	float4x4 cView;
	float4x4 cViewInverse;
	float4x4 cViewProj;
	float4 cFarClip;
	float4 cNearClip;
}

cbuffer cPerObject : register(b2)
{
	float4x4 cWorld;
	float4x4 cWorldViewProj;

#ifdef SKINNED
	float4x4 cSkinMatrices[100];
#endif
}