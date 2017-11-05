workspace "FluxEngine"
	filename "FluxEngine_%{_ACTION}"
	basedir "../"
	configurations { "Debug", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"

    filter { "platforms:x64" }
		architecture "x64"
		defines {"x64", "PLATFORM_WINDOWS"}

	filter { "platforms:x86" }
		architecture "x32"
		defines {"x86", "PLATFORM_WINDOWS"}	

	project "FluxEngine"
		filename "FluxEngine_%{_ACTION}"
		location "../FluxEngine"
		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader "stdafx.h"
		pchsource "../FluxEngine/stdafx.cpp"
		kind "StaticLib"
		characterset ("MBCS")
		defines { "_CONSOLE", "D3D11" }
		flags {"FatalWarnings"}
		language "C++"

		files
		{ 
			"../FluxEngine/**.h",
			"../FluxEngine/**.hpp",
			"../FluxEngine/**.cpp",
			"../FluxEngine/**.inl"
		}

		includedirs 
		{ 
			"$(ProjectDir)",
			"../Libraries/ImgUI/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Zlib/include",
		}

		libdirs
		{
			"../Libraries/Imgui/lib/%{cfg.platform}",
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
		}

		filter { "configurations:Debug" }
			runtime "Debug"
		 	defines { "_DEBUG" }
		 	flags {  }
		 	symbols "On"
		 	optimize "Off"

		filter { "configurations:Release" }
		 	runtime "Release"
			defines { "NDEBUG" }
		 	flags {  }
		 	symbols "Off"
		 	optimize "Full"

	project "FluxGame"
		filename "FluxGame_%{_ACTION}"
		location "../FluxGame"
		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader "stdafx.h"
		pchsource "../FluxGame/stdafx.cpp"
		kind "WindowedApp"
		characterset ("MBCS")
		defines { "_CONSOLE" }
		flags {"FatalWarnings"}
		language "C++"

		links {"FluxEngine"}

		files
		{ 
			"../FluxGame/**.h",
			"../FluxGame/**.hpp",
			"../FluxGame/**.cpp",
			"../FluxGame/**.inl"
		}

		includedirs 
		{ 
			"$(ProjectDir)/../FluxEngine",
			"$(ProjectDir)",
			"../Libraries/ImgUI/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Zlib/include",
		}

		libdirs
		{
			"../Libraries/Imgui/lib/%{cfg.platform}",
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
		}

		filter { "configurations:Debug" }
			runtime "Debug"
		 	defines { "_DEBUG" }
		 	flags {  }
		 	symbols "On"
		 	optimize "Off"
		 	postbuildcommands
			{ 
				"copy \"..\\Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib_DEBUG.dll\" \"$(OutDir)\" /y /D",
			}

		filter { "configurations:Release" }
		 	runtime "Release"
			defines { "NDEBUG" }
		 	flags {  }
		 	symbols "Off"
		 	optimize "Full"
		 	postbuildcommands
			{ 
				"copy \"..\\Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib.dll\" \"$(OutDir)\" /y /D",
				"dir \"$(SolutionDir)FluxGame\\Resources\\\" /s /b /a-d > \"$(SolutionDir)\\FluxPak\\ResponseFile.txt\"",
				"\"$(SolutionDir)FluxPak\\PakFileCompression.exe\" -r \"$(SolutionDir)\\FluxPak\\ResponseFile.txt\" -p \"$(SolutionDir)FluxGame\\Resources.pak\" -b \"$(SolutionDir)FluxGame/Resources/",
			}

		filter { "platforms:x64" }
			postbuildcommands
			{ 
				"copy \"..\\Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\" /y /D",
			}

		filter { "platforms:x86" }
			postbuildcommands
			{ 
				"copy \"..\\Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\" /y /D",
			}

newaction {
		trigger     = "clean",
		description = "Remove all binaries and generated files",

		execute = function()
			os.rmdir("../Build")
			os.rmdir("../ipch")
		end
	}