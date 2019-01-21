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
#pragma endregion STL

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifdef MODULE_PHYSX
#include <PxPhysicsAPI.h>
#endif

#include "External/Imgui/imgui.h"

#pragma region
//Engine core include
#include "Core/Types.h"
#include "Misc/Misc.h"
#include "Core/BitField.h"
#include "Helpers/StringHash.h"
#include "Helpers/Utility.h"
#include "Math/MathTypes.h"
#include "Math/MathHelp.h"
#include "Helpers/Paths.h"
#include "Helpers/SmartInterface.h"
#include "Helpers/LogMacros.h"
#include "Helpers/AssertMacros.h"
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

#include "FileSystem/FileSystem.h"
#include "FileSystem/File/File.h"

#include "Content/ResourceManager.h"
#pragma endregion ENGINE INCLUDES