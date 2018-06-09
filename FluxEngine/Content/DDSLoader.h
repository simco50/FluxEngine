#pragma once

#define MAKEFOURCC(a, b, c, d) (unsigned int)((unsigned char)a | (unsigned char)b << 8 | (unsigned char)c << 16 | (unsigned char)d << 24)

namespace DDS
{
	// .DDS subheader.
#pragma pack(push,1)
	struct DDSPixelFormatHeader
	{
		uint32 dwSize;
		uint32 dwFlags;
		uint32 dwFourCC;
		uint32 dwRGBBitCount;
		uint32 dwRBitMask;
		uint32 dwGBitMask;
		uint32 dwBBitMask;
		uint32 dwABitMask;
	};
#pragma pack(pop)

	// .DDS header.
#pragma pack(push,1)
	struct DDSFileHeader
	{
		uint32 dwSize;
		uint32 dwFlags;
		uint32 dwHeight;
		uint32 dwWidth;
		uint32 dwLinearSize;
		uint32 dwDepth;
		uint32 dwMipMapCount;
		uint32 dwReserved1[11];
		DDSPixelFormatHeader ddpf;
		uint32 dwCaps;
		uint32 dwCaps2;
		uint32 dwCaps3;
		uint32 dwCaps4;
		uint32 dwReserved2;
	};
#pragma pack(pop)

	// .DDS 10 header.
#pragma pack(push,1)
	struct DDS10FileHeader
	{
		uint32 dxgiFormat;
		uint32 resourceDimension;
		uint32 miscFlag;
		uint32 arraySize;
		uint32 reserved;
	};
#pragma pack(pop)

	enum
	{
		DDS_DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DDS_DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 26,
		DDS_DXGI_FORMAT_BC1_UNORM = 71,
		DDS_DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DDS_DXGI_FORMAT_BC2_UNORM = 74,
		DDS_DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DDS_DXGI_FORMAT_BC3_UNORM = 77,
		DDS_DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	};

	enum
	{
		DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400U,
		DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800U,
		DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000U,
		DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000U,
		DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000U,
		DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000U,
		DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U,
	};

	enum
	{
		DDSCAPS_COMPLEX = 0x00000008U,
		DDSCAPS_TEXTURE = 0x00001000U,
		DDSCAPS_MIPMAP = 0x00400000U,
		DDSCAPS2_VOLUME = 0x00200000U,
		DDSCAPS2_CUBEMAP = 0x00000200U,
	};
}