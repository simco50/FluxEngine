#ifdef COMPILE_VS

cbuffer cPerFrameVS : register(b0)
{
	float cDeltaTimeVS;
	float cElapsedTimeVS;
}

cbuffer cPerViewVS : register(b1)
{
	float4x4 cViewMatrixVS;
	float4x4 cViewInverseMatrixVS;
	float4x4 cViewProjectionMatrixVS;
	float4 cFarClip;
	float4 cNearClip;
}

cbuffer cPerObject : register(b2)
{
	float4x4 cWorldMatrixVS;
}

#endif

#ifdef COMPILE_PS

cbuffer cPerFrameVS : register(b0)
{
	float cDeltaTimeVS;
}

cbuffer cPerViewVS : register(b1)
{
	float4x4 cViewMatrixVS;
	float4x4 cViewInverseMatrixVS;
	float4x4 cViewProjectionMatrixVS;
}

cbuffer cPerObject : register(b2)
{
	float4x4 cWorldMatrixVS;
}

#endif