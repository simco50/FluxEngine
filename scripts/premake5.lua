require "Modules"
require "Utility"

local engineName = "FluxEngine"
local gameName = "FluxGame"

workspace (engineName)
	filename (engineName)
	basedir "../"
	configurations { "Debug", "Test", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"
    rtti "Off"
	characterset ("MBCS")
	defines { "_CONSOLE", "THREADING"}
	flags {"FatalWarnings"}
	language "C++"
	startproject (gameName)

    filter { "platforms:x64" }
		architecture "x64"
		defines {"x64", "PLATFORM_WINDOWS"}

	filter { "platforms:x86" }
		architecture "x32"
		defines {"x86", "PLATFORM_WINDOWS"}	

	filter { "configurations:Debug" }
		runtime "Debug"
	 	defines { "FLUX_DEBUG", "_DEBUG", "PROFILING" }
	 	symbols "On"
	 	optimize "Off"

 	filter { "configurations:Test" }
		runtime "Release"
	 	defines { "FLUX_TEST", "NDEBUG", "PROFILING" }
	 	symbols "On"
	 	optimize "Full"

	filter { "configurations:Release" }
	 	runtime "Release"
		defines { "FLUX_RELEASE", "NDEBUG" }
	 	symbols "Off"
	 	optimize "Full"

	project (engineName)
		filename (engineName)
		location ("../" .. engineName)
		targetdir ("../Build/" .. gameName .. "_$(Platform)_$(Configuration)")
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader (engineName .. ".h")
		pchsource ("../" .. engineName .. "/" .. engineName .. ".cpp")
		kind "StaticLib"

		AddSourceFiles(engineName)
		
		includedirs 
		{ 
			"$(ProjectDir)"
		}

		AddPhysX(false)
		AddFmod(false)
		AddSQLite(false)
		AddZlib(false)
		AddSDL2(false)
		AddAssimp(false)
		AddDX11(false)
		AddWininet(false)

		filter "files:**/External/**.*"
			flags "NoPCH"

	project (gameName)
		filename (gameName)
		location ("../" .. gameName)
		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader (gameName .. ".h")
		pchsource ("../" .. gameName .. "/" .. gameName .. ".cpp")
		kind "WindowedApp"

		links { (engineName) }

		AddSourceFiles(gameName)

		includedirs 
		{ 
			("../" .. engineName)
		}

		AddPhysX(true)
		AddFmod(true)
		AddSQLite(true)
		AddZlib(true)
		AddSDL2(true)
		AddAssimp(true)
		AddDX11(true)
		AddWininet(true)