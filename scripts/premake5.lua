require "Modules"
require "Utility"

local engineName = "FluxEngine"
local gameName = "FluxGame"

ROOT = "../"

workspace (engineName)
	filename (engineName)
	basedir (ROOT)
	configurations { "Debug", "Test", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"
    rtti "Off"
	characterset ("MBCS")
	defines { "_CONSOLE", "THREADING", "PLATFORM_WINDOWS"}
	flags {"FatalWarnings", "MultiProcessorCompile"}
	language "C++"
	startproject (gameName)

    filter { "platforms:x64" }
		architecture "x86_64"
		defines {"x64", "_AMD64_"}

	filter { "platforms:x86" }
		architecture "x86"
		defines {"x86", "_X86_"}	

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

	filter {}

	project (engineName)
		SetupTarget(engineName, "StaticLib")
		AddSourceFiles(engineName)
		
		includedirs { "$(ProjectDir)" }

		filter { "files:**/External/**.*"}
			flags "NoPCH"

		filter {}

		AddPhysX(false)
		AddFmod(false)
		AddSQLite(false)
		AddZlib(false)
		AddSDL2(false)
		AddAssimp(false)
		AddDX11(false)
		AddWininet(false)

	project (gameName)
		SetupTarget(gameName, "WindowedApp")

		AddSourceFiles(gameName)

		links { (engineName) }
		includedirs { (ROOT .. engineName) }

		AddPhysX(true)
		AddFmod(true)
		AddSQLite(true)
		AddZlib(true)
		AddSDL2(true)
		AddAssimp(true)
		AddDX11(true)
		AddWininet(true)