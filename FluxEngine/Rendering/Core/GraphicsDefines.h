#pragma once

namespace ShaderConstant
{
#define DEFINE_SHADER_PARAMETER(variableName, name) constexpr StringHash variableName(name);

	//Frame variables
	DEFINE_SHADER_PARAMETER(cElapsedTime, "cElapsedTime");
	DEFINE_SHADER_PARAMETER(cDeltaTime, "cDeltaTime");
	DEFINE_SHADER_PARAMETER(cLights, "cLights");

	//Camera variables
	DEFINE_SHADER_PARAMETER(cViewProj, "cViewProj");
	DEFINE_SHADER_PARAMETER(cView, "cView");
	DEFINE_SHADER_PARAMETER(cViewInverse, "cViewInverse");
	DEFINE_SHADER_PARAMETER(cNearClip, "cNearClip");
	DEFINE_SHADER_PARAMETER(cFarClip, "cFarClip");

	//Material variables
	DEFINE_SHADER_PARAMETER(cColor, "cColor");

	//Model variabl
	DEFINE_SHADER_PARAMETER(cWorld, "cWorld");
	DEFINE_SHADER_PARAMETER(cWorldViewProj, "cWorldViewProj");
	DEFINE_SHADER_PARAMETER(cSkinMatrices, "cSkinMatrices");
	DEFINE_SHADER_PARAMETER(cSkinDualQuaternions, "cSkinDualQuaternions");

#undef DEFINE_SHADER_PARAMETER
};

namespace GraphicsConstants
{
	constexpr int MAX_VERTEX_BUFFERS = 2;
	constexpr int MAX_RENDERTARGETS = 2;
	constexpr int MAX_LIGHTS = 20;
	constexpr int MAX_BONES = 50;
	constexpr int MAX_BONES_PER_VERTEX = 4;
	constexpr int MAX_UV_CHANNELS = 3;
}

enum class ShaderType
{
	VertexShader = 0,
	PixelShader,
	GeometryShader,
	DomainShader,
	HullShader,

	ComputeShader,
	MAX,
};

//The primitive topology for rendering
enum class PrimitiveType : unsigned char
{
	TRIANGLELIST = 0,
	POINTLIST,
	TRIANGLESTRIP,
	LINELIST,
	PATCH_CP_3,
};

//The window type
enum class WindowType
{
	WINDOWED = 0,
	BORDERLESS,
	FULLSCREEN,
};

//The fillmode of the rasterizer state
enum class FillMode
{
	SOLID = 0,
	WIREFRAME,
};

//The cull mode of the rasterizer state
enum class CullMode
{
	FRONT = 0,
	BACK,
	NONE,
};

//The blend mode of the blend state
enum class BlendMode
{
	REPLACE = 0,
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
enum class ColorWrite
{
	NONE = 0,
	RED = 1 << 0,
	GREEN = 1 << 1,
	BLUE = 1 << 2,
	ALPHA = 1 << 3,
	ALL = RED | GREEN | BLUE | ALPHA,
};
DEFINE_ENUM_FLAG_OPERATORS(ColorWrite);

//Compare mode for rendering state
enum class CompareMode
{
	ALWAYS = 0,
	EQUAL,
	NOTEQUAL,
	LESS,
	LESSEQUAL,
	GREATER,
	GREATEREQUAL,
	UNDEFINED,
};

//Sntecil operation for the depth stencil state
enum class StencilOperation
{
	KEEP = 0,
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
	Lights = 3,
	Custom = 4,
	MAX
};

//Predefined texture slot per convention
enum class TextureSlot
{
	Diffuse = 0,
	Normal = 1,
	Specular = 2,
	Volume = 3,
	Cube = 3,
	Lights = 4,
	Shadow = 5,
	PositionMorph = 6,
	NormalMorph = 7,
	MAX = 8
};

enum class TextureFilter
{
	MIN_MAG_MIP_POINT = 0,
	MIN_MAG_POINT_MIP_LINEAR,
	MIN_POINT_MAG_LINEAR_MIP_POINT,
	MIN_POINT_MAG_MIP_LINEAR,
	MIN_LINEAR_MAG_MIP_POINT,
	MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MIN_MAG_LINEAR_MIP_POINT,
	MIN_MAG_MIP_LINEAR,
	ANISOTROPIC,
	COMPARISON_MIN_MAG_MIP_POINT,
	COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
	COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
	COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
	COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
	COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
	COMPARISON_MIN_MAG_MIP_LINEAR,
	COMPARISON_ANISOTROPIC,
	MINIMUM_MIN_MAG_MIP_POINT,
	MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
	MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
	MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
	MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
	MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
	MINIMUM_MIN_MAG_MIP_LINEAR,
	MINIMUM_ANISOTROPIC,
	MAXIMUM_MIN_MAG_MIP_POINT,
	MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
	MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
	MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
	MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
	MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
	MAXIMUM_MIN_MAG_MIP_LINEAR,
	MAXIMUM_ANISOTROPIC,
};

//Bitflag to clear textures
enum class ClearFlags
{
	NONE = 0,
	Depth = 1 << 0,
	Stencil = 1 << 1,
	RenderTarget = 1 << 2,
	All = Depth | Stencil | RenderTarget,
};
DEFINE_ENUM_FLAG_OPERATORS(ClearFlags);