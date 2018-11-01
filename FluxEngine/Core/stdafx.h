#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(DEBUG) || defined(_DEBUG)
#include <vld.h>
#endif

#pragma region

//Standard Library
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <map>
#include <exception>
#include <sstream>
#include <memory>
#include <fstream>

using namespace std;

#pragma endregion STL


#pragma region

//DirectX
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

//DirectXMath
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
using namespace DirectX;

//**EFFECTS11 (Helper for loading Effects (D3DX11))
//https://fx11.codeplex.com/
#include "d3dx11effect.h" //[AdditionalLibraries/DX_Effects11/include/d3dx11effect.h]

#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DxEffects11_Debug.lib")
#else
#pragma comment(lib, "DxEffects11_Release.lib")
#endif

//*DXTEX (Helper for loading Textures (D3DX11))
//http://directxtex.codeplex.com/
#include "DirectXTex.h"
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DirectXTex_Debug.lib")
#else
#pragma comment(lib, "DirectXTex_Release.lib")
#endif

#pragma endregion D3D

#pragma region
#include <flex.h>
#include <flexExt.h>

#ifdef x86

#ifdef _DEBUG
#pragma comment(lib, "flexDebug_x86.lib")
#pragma comment(lib, "flexExtDebug_x86.lib")
#else
#pragma comment(lib, "flexExtRelease_x86.lib")
#pragma comment(lib, "flexRelease_x86.lib")
#endif

#else

#ifdef _DEBUG
#pragma comment(lib, "flexDebug_x64.lib")
#pragma comment(lib, "flexExtDebug_x64.lib")
#else
#pragma comment(lib, "flexExtRelease_x64.lib")
#pragma comment(lib, "flexRelease_x64.lib")
#endif

#endif

#pragma endregion FLEX

#pragma region

//FMOD
#include "fmod.hpp"
#pragma warning(push)
#pragma warning(disable: 4505) //Unreferenced local function removed (FMOD_ErrorString)
#include "fmod_errors.h"
#pragma warning(pop)

#ifdef x86

#pragma comment(lib, "fmod_vc.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL_vc.lib")
#endif

#else

#pragma comment(lib, "fmod64_vc.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL64_vc.lib")
#endif

#endif

#pragma endregion FMOD

#pragma region
//XINPUT
#include <Xinput.h>
#pragma comment(lib, "XINPUT9_1_0.LIB")
#pragma endregion XINPUT

#pragma region
#undef ERROR
//Engine core include
//#include "../Math/FluxMath.h"
#include "../Debugging/DebugLog.h"
#include "GeneralStructs.h"
#include "GameTimer.h"
#include "../Helpers/VertexStructures.h"
#include "../Managers/InputManager.h"
#include "../Content/ResourceManager.h"
#include "../Helpers/SmartInterface.h"
#include "../Debugging/PerfTimer.h"
#include "../Helpers/ShaderStructs.h"
using namespace Smart_COM;
#include "../Math/SimpleMath.h"
using namespace DirectX::SimpleMath;
#pragma endregion ENGINE INCLUDES

#pragma region

//Macros
#define HR(command)\
{HRESULT r = command;\
wstringstream stream;\
stream << typeid(*this).name() << "::" << __func__ << "()" << endl;\
stream << "Line: " << __LINE__  << endl;\
stream << "Action: " << #command ;\
DebugLog::LogHRESULT(stream.str(), r);}

#define BIND_AND_CHECK_SEMANTIC(variable, semantic, as)\
{variable = m_pEffect->GetVariableBySemantic(semantic)->as();\
wstringstream stream;\
stream << L"Variable with semantic '" << semantic << "' not found";\
if(! variable->IsValid()) DebugLog::Log(stream.str(), LogType::ERROR);}

#define BIND_AND_CHECK_NAME(variable, name, as)\
variable = m_pEffect->GetVariableByName(#name)->as();\
if(! variable->IsValid()) DebugLog::LogFormat(LogType::ERROR, L"Variable with name '%s' not found", L#name);

#define ERROR_TEXTURE L"./Resources/Textures/ErrorTexture.jpg"

//Utility methods
template<typename T>
inline void SafeDelete(T*& object)
{
	if(object)
	{
		delete object;
		object = nullptr;
	}
}

template<typename T>
inline void SafeRelease(T*& object)
{
	if (object)
	{
		object->Release();
		object = nullptr;
	}
}

#pragma endregion HELPERS