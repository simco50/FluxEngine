#pragma once

enum class PrimitiveType : unsigned char
{
	TRIANGLELIST,
	POINTLIST,
	TRIANGLESTRIP,
	UNDEFINED,
};

enum class WindowType : unsigned char
{
	WINDOWED,
	BORDERLESS,
	FULLSCREEN,
};

enum class FillMode : unsigned char
{
	SOLID,
	WIREFRAME,
	UNDEFINED,
};

enum class CullMode : unsigned char
{
	FRONT,
	BACK,
	NONE,
	UNDEFINED,
};

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

enum class ColorWrite : unsigned char
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA,
};

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

enum class StencilOperation : unsigned char
{
	KEEP,
	ZERO,
	REF,
	INCR,
	DECR
};