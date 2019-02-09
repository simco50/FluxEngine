require "Modules"
require "Utility"

engineName = "FluxEngine"
gameName = "FluxGame"

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
	--flags { "FatalWarnings" }
	flags { "MultiProcessorCompile" }
	language "C++"
	startproject (gameName)
	systemversion "latest"

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

		AddModules(false)

	project (gameName)
		SetupTarget(gameName, "WindowedApp")

		AddSourceFiles(gameName)

		links { (engineName) }
		includedirs { (ROOT .. engineName) }

		AddModules(true)

newoption {
	trigger     = "gfxapi",
	value       = "API",
	description = "Choose a particular 3D API for rendering",
	default     = "d3d11",
	allowed = {
		{ "d3d11", "DirectX 11" },
		{ "d3d12", "DirectX 12" },
	}
}