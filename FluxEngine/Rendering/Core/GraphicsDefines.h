#pragma once

namespace GraphicsConstants
{
	static const int MAX_VERTEX_BUFFERS = 4;
}

enum class ShaderType : unsigned int
{
	VertexShader = 0,
	PixelShader,
	GeometryShader,
	ComputeShader,
	MAX,
};

//The primitive topology for rendering
enum class PrimitiveType : unsigned char
{
	TRIANGLELIST,
	POINTLIST,
	TRIANGLESTRIP,
	UNDEFINED,
};

//The windwo type
enum class WindowType : unsigned char
{
	WINDOWED,
	BORDERLESS,
	FULLSCREEN,
};

//The fillmode of the rasterizer state
enum class FillMode : unsigned char
{
	SOLID,
	WIREFRAME,
	UNDEFINED,
};

//The cull mode of the rasterizer state
enum class CullMode : unsigned char
{
	FRONT,
	BACK,
	NONE,
	UNDEFINED,
};

//The blend mode of the blend state
enum class BlendMode : unsigned char
{
	REPLACE,
	ADD,
	MULTIPLY,
	ALPHA,
	ADDALPHA,
	PREMULALPHA,
	INVDESTALPHA,
	SUBTRACT,
	SUBTRACTALPHA,
	UNDEFINED,
};

//Color write mode of the blend state
enum class ColorWrite : unsigned char //MAX 5!
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA,
};

//Compare mode for rendering state
enum class CompareMode : unsigned char
{
	ALWAYS,
	EQUAL,
	NOTEQUAL,
	LESS,
	LESSEQUAL,
	GREATER,
	GREATEREQUAL,
	UNDEFINED,
};

//Sntecil operation for the depth stencil state
enum class StencilOperation : unsigned char
{
	KEEP,
	ZERO,
	REF,
	INCR,
	DECR
};

//The convention for the shader constant buffer register
enum class ShaderParameterType
{
	PerFrame = 0,
	PerView = 1,
	PerObject = 2,
	Custom = 3,
	MAX
};