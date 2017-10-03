#pragma once

#ifdef _DEBUG
#define PROFILING
#endif

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(_DEBUG)
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
#include <sstream>
#include <memory>
#include <fstream>

using namespace std;

#pragma endregion STL


#ifdef PHYSX

#include <PxPhysicsAPI.h>
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "PhysX3DEBUG_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib, "PxFoundationDEBUG_x86.lib")
#pragma comment(lib, "PxTaskDEBUG_x86.lib")
#pragma comment(lib, "PxPvdSDKDEBUG_x86.lib")
#else
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PxFoundation_x86.lib")
#pragma comment(lib, "PxTask_x86.lib")
#pragma comment(lib, "PxPvdSDK_x86.lib")
#endif
using namespace physx;

#endif

#pragma region

#include <imgui.h>
#ifdef _DEBUG
#pragma comment(lib, "imgui_Debug.lib")
#else
#pragma comment(lib, "imgui_Release.lib")
#endif

#pragma endregion IMGUI

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

#include "d3dx11effect.h"
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DxEffects11_Debug.lib")
#else 
#pragma comment(lib, "DxEffects11_Release.lib")
#endif

#include "DirectXTex.h"
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DirectXTex_Debug.lib")
#else 
#pragma comment(lib, "DirectXTex_Release.lib")
#endif

#pragma endregion D3D

#ifdef FLEX
#pragma region
#include <NvFlex.h>
#include <NvFlexExt.h>

#ifdef _DEBUG
#pragma comment(lib, "NvFlexDebugD3D_x86.lib")
#pragma comment(lib, "NvFlexExtDebugD3D_x86.lib")
#else
#pragma comment(lib, "NvFlexReleaseD3D_x86.lib")
#pragma comment(lib, "NvFlexExtReleaseCUDA_x86.lib")
#endif
#pragma endregion FLEX
#endif

#pragma region

//FMOD
#include "fmod.hpp"
#pragma warning(push)
#pragma warning(disable: 4505) //Unreferenced local function removed (FMOD_ErrorString)
#include "fmod_errors.h"
#pragma warning(pop)
#if x64
#pragma comment(lib, "fmod64_vc.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL64_vc.lib")
#endif
#elif defined(x86)
#pragma comment(lib, "fmod_vc.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL_vc.lib")
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
#include "Helpers/SmartInterface.h"
#include "Helpers/LogMacros.h"
#include "Helpers/AssertMacros.h"
#include "Helpers/Utility.h"
#include "Materials/VertexStructures.h"
#include "Rendering/Core/ShaderStructs.h"

#include "Debugging/Console.h"
#include "Debugging/Profiler.h"

#include "Core/GeneralStructs.h"
#include "Core/GameTimer.h"
#include "Core/InputEngine.h"

#include "Content/ResourceManager.h"

#include "Audio/AudioEngine.h"

#include "Math/MathHelp.h"
#include "Math/MathTypes.h"

using namespace Smart_COM;
#include "Math/SimpleMath.h"
using namespace DirectX::SimpleMath;
#pragma endregion ENGINE INCLUDES

#pragma region


#pragma endregion HELPERS