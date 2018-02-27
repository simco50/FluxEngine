workspace "FluxEngine"
	filename "FluxEngine_%{_ACTION}"
	basedir "../"
	configurations { "Debug", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"
    rtti "Off"
	characterset ("MBCS")
	defines { "_CONSOLE", "PHYSX", "PROFILING", "THREADING", "GRAPHICS_D3D11" }
	flags {"FatalWarnings"}
	language "C++"

    filter { "platforms:x64" }
		architecture "x64"
		defines {"x64", "PLATFORM_WINDOWS"}

	filter { "platforms:x86" }
		architecture "x32"
		defines {"x86", "PLATFORM_WINDOWS"}	

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

	project "FluxEngine"
		filename "FluxEngine_%{_ACTION}"
		location "../FluxEngine"
		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader "FluxEngine.h"
		pchsource "../FluxEngine/FluxEngine.cpp"
		kind "StaticLib"

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
			"../Libraries/PhysX 3.4/PhysX_3.4/include",
			"../Libraries/PhysX 3.4/PxShared/include",
		}

		libdirs
		{
			"../Libraries/Imgui/lib/%{cfg.platform}",
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PhysX_3.4/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PxShared/lib/%{cfg.platform}",
		}

	project "FluxGame"
		filename "FluxGame_%{_ACTION}"
		location "../FluxGame"
		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader "FluxGame.h"
		pchsource "../FluxGame/FluxGame.cpp"
		kind "WindowedApp"

		links { "FluxEngine" }

		files
		{ 
			"../FluxGame/**.h",
			"../FluxGame/**.hpp",
			"../FluxGame/**.cpp",
			"../FluxGame/**.inl"
		}

		includedirs 
		{ 
			"../FluxEngine",
			"../Libraries/ImgUI/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Zlib/include",
			"../Libraries/PhysX 3.4/PhysX_3.4/include",
			"../Libraries/PhysX 3.4/PxShared/include",
		}

		libdirs
		{
			"../Libraries/Imgui/lib/%{cfg.platform}",
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PhysX_3.4/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PxShared/lib/%{cfg.platform}",
		}

		postbuildcommands
		{ 
			"xcopy \"$(ProjectDir)Logo.ico\" \"$(OutDir)\" /Y /D",
		}

		filter { "configurations:Debug" }
		 	postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib_DEBUG.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "configurations:Release" }
		 	postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib.dll\" \"$(OutDir)\" /Y /D",
				"dir \"$(SolutionDir)FluxGame\\Resources\\\" /s /b /a-d > \"$(SolutionDir)\\FluxPak\\ResponseFile.txt\"",
				"\"$(SolutionDir)FluxPak\\PakFileCompression.exe\" -r \"$(SolutionDir)\\FluxPak\\ResponseFile.txt\" -p \"$(SolutionDir)FluxGame\\Resources.pak\" -b \"$(SolutionDir)FluxGame/Resources/",
			}

		filter { "platforms:x64" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\" /F /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\nvToolsExt64_1.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "platforms:x86" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\" /F /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\nvToolsExt32_1.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "configurations:Debug", "platforms:x86" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3CommonDEBUG_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3DEBUG_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxFoundationDEBUG_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxPvdSDKDEBUG_x86.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "configurations:Release", "platforms:x86" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3Common_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxFoundation_x86.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxPvdSDK_x86.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "configurations:Debug", "platforms:x64" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3CommonDEBUG_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3DEBUG_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxFoundationDEBUG_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxPvdSDKDEBUG_x64.dll\" \"$(OutDir)\" /Y /D",
			}

		filter { "configurations:Release", "platforms:x64" }
			postbuildcommands
			{ 
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3Common_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxFoundation_x64.dll\" \"$(OutDir)\" /Y /D",
				"xcopy \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxPvdSDK_x64.dll\" \"$(OutDir)\" /Y /D",
			}

newaction {
	trigger     = "clean",
	description = "Remove all binaries and generated files",

	execute = function()
		os.rmdir("../Build")
		os.rmdir("../ipch")
	end
}