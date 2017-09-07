workspace "FluxEngine"
	basedir "../"
	configurations { "Debug", "Release" }
    platforms "x86"
    warnings "Extra"

	filter "configurations:Debug"
			runtime "Debug"
		 	defines { "_DEBUG" }
		 	flags {  }
		 	symbols "On"
		 	optimize "Off"

	filter "configurations:Release"
		 	runtime "Release"
			defines { "NDEBUG" }
		 	flags {  }
		 	symbols "Off"
		 	optimize "Full"

	project "FluxEngine"

		location "../FluxEngine/"

		targetdir "../Build/%{cfg.buildcfg}"
		objdir "../Build/Intermediate/%{cfg.buildcfg}"

		pchheader "stdafx.h"
		pchsource "../FluxEngine/stdafx.cpp"
		kind "ConsoleApp"
		characterset ("MBCS")
		defines { "WIN32", "_CONSOLE" }

		files
		{ 
			"../FluxEngine/stdafx.cpp",
			"../FluxEngine/stdafx.h",

			"../FluxEngine/**.h",
			"../FluxEngine/**.hpp",
			"../FluxEngine/**.cpp"
		}

		includedirs 
		{ 
			"$(ProjectDir)",
			"../Libraries/ImgUI/include",
			"../Libraries/DX_Effects11/include",
			"../Libraries/DX_Tex/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Nvidia Flex 1.1.0/include",
			"../Libraries/PhysX 3.4/Include",
		}

		excludes
		{
			"../FluxEngine/Physics/**.*",	
			"../FluxEngine/Materials/Flex/**.*",
			"../FluxEngine/FlexManager.*"
		}

		libdirs
		{
			"../Libraries/Imgui/lib",
			"../Libraries/DX_Tex/lib/",
			"../Libraries/Fmod/lib",
			"../Libraries/DX_Effects11/lib",
			"../Libraries/PhysX 3.4/lib/vc14win32",
			"../Libraries/Nvidia Flex 1.1.0/lib/win32"
		}

		postbuildcommands 
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\fmod.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\win32\\cudart32_80.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\nvToolsExt32_1.dll\" \"$(OutDir)\" /y /D"
		}

		configuration "Debug32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\win32\\NvFlexDebugD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\win32\\NvFlexExtDebugD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3DEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3CommonDEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxPvdSDKDEBUG_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxFoundationDEBUG_x86.dll\" \"$(OutDir)\" /D",
		 	}

		configuration "Release32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\win32\\NvFlexReleaseD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\win32\\NvFlexExtReleaseD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3Common_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxPvdSDK_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxFoundation_x86.dll\" \"$(OutDir)\" /D",
		 	}