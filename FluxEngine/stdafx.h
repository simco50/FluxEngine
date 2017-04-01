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
#pragma endregion PhysX

#include <imgui.h>
#pragma comment(lib, "imgui.lib")

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
#pragma comment(lib, "DxEffects11_vc14_Debug.lib")
#else 
#pragma comment(lib, "DxEffects11_vc14_Release.lib")
#endif

//*DXTEX (Helper for loading Textures (D3DX11))
//http://directxtex.codeplex.com/
#include "DirectXTex.h"
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "DxTex_vc14_Debug.lib")
#else 
#pragma comment(lib, "DxTex_vc14_Release.lib")
#endif

#pragma endregion D3D

#pragma region
#include <flex.h>
#include <flexExt.h>

#ifdef _DEBUG
#pragma comment(lib, "flexDebug_x86.lib")
#pragma comment(lib, "flexExtDebug_x86.lib")
#else
#pragma comment(lib, "flexExtRelease_x86.lib")
#pragma comment(lib, "flexRelease_x86.lib")
#endif
#pragma endregion FLEX

#pragma region

//FMOD
#include "fmod.hpp"
#pragma warning(push)
#pragma warning(disable: 4505) //Unreferenced local function removed (FMOD_ErrorString)
#include "fmod_errors.h"
#pragma warning(pop)

#pragma comment(lib, "fmod_vc.lib")
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "fmodL_vc.lib")
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
#include "Debugging/Console.h"
#include "Core/GeneralStructs.h"
#include "Core/GameTimer.h"
#include "Helpers/VertexStructures.h"
#include "Managers/InputManager.h"
#include "Content/ResourceManager.h"
#include "Helpers/SmartInterface.h"
#include "Debugging/PerfTimer.h"
#include "Helpers/ShaderStructs.h"
using namespace Smart_COM;
#include "Math/SimpleMath.h"
using namespace DirectX::SimpleMath;
#pragma endregion ENGINE INCLUDES

#pragma region

#include "Helpers/Utility.h"

#pragma endregion HELPERS