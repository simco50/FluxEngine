#pragma once

//Force to disable RTTI because we have our own system
#ifdef _CPPRTTI
#error RTTI is enabled. Disable using "/GR-"
#endif

#if !defined(_MBCS)
#error Character set must be MBCS
#endif

#pragma region
//Standard Library
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <sstream>
#include <memory>
#include <iomanip>
#include <queue>
#include <limits>
#pragma endregion STL

#include <SDL.h>
#pragma comment(lib, "sdl2.lib")
#pragma comment(lib, "sdl2main.lib")

#ifdef PHYSX
#include <PxPhysicsAPI.h>
#if defined(x64) && defined(_DEBUG)
#pragma comment(lib, "PhysX3DEBUG_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PxFoundationDEBUG_x64.lib")
#pragma comment(lib, "PxTaskDEBUG_x64.lib")
#pragma comment(lib, "PxPvdSDKDEBUG_x64.lib")
#elif defined(x64) && defined(NDEBUG)
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PxFoundation_x64.lib")
#pragma comment(lib, "PxTask_x64.lib")
#pragma comment(lib, "PxPvdSDK_x64.lib")
#elif defined(x86) && defined(_DEBUG)
#pragma comment(lib, "PhysX3DEBUG_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PxFoundationDEBUG_x86.lib")
#pragma comment(lib, "PxTaskDEBUG_x86.lib")
#pragma comment(lib, "PxPvdSDKDEBUG_x86.lib")
#elif defined(x86) && defined(NDEBUG)
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PxFoundation_x86.lib")
#pragma comment(lib, "PxTask_x86.lib")
#pragma comment(lib, "PxPvdSDK_x86.lib")
#endif
#endif

#pragma region
#include "External/Imgui/imgui.h"
#pragma endregion IMGUI

#pragma region
//DirectXMath
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <dxgi.h>
#include <d3d11.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#include <DirectXColors.h>
using namespace DirectX;
#include "External/SimpleMath/SimpleMath.h"
using namespace DirectX::SimpleMath;
#pragma endregion D3D

#pragma region
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
//Engine core include
#include "Core/Types.h"
#include "Misc/Misc.h"
#include "Helpers/Paths.h"
#include "Helpers/SmartInterface.h"
#include "Helpers/LogMacros.h"
#include "Helpers/AssertMacros.h"
#include "Helpers/Utility.h"
#include "Misc/TimeSpan.h"
#include "Misc/DateTime.h"

#include "Diagnostics/Console.h"
#include "Diagnostics/Profiler.h"

#include "Core/TypeInfo.h"
#include "Core/Context.h"
#include "Core/Object.h"
#include "Core/Delegates.h"
#include "Core/GameTimer.h"

#include "Audio/AudioEngine.h"

#include "Math/MathHelp.h"
#include "Math/MathTypes.h"

#include "FileSystem/FileSystem.h"
#include "FileSystem/File/File.h"

#include "Content/ResourceManager.h"
#pragma endregion ENGINE INCLUDES