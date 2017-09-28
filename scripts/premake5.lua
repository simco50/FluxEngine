solution ("FluxEngine")
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

		targetdir "../Build/$(ProjectName)_$(Platform)_$(Configuration)"
		objdir "!../Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)"

		pchheader "stdafx.h"
		pchsource "../FluxEngine/stdafx.cpp"
		kind "WindowedApp"
		characterset ("MBCS")
		defines { "WIN32", "_CONSOLE" }
		flags {"FatalWarnings"}

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
			"../Libraries/DX_Effects11/include",
			"../Libraries/DX_Tex/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Nvidia Flex 1.1.0/include",
			"../Libraries/PhysX 3.4/Include"
		}

		libdirs
		{
			"../Libraries/Imgui/lib/x86",
			"../Libraries/DX_Tex/lib/x86",
			"../Libraries/Fmod/lib/x86",
			"../Libraries/DX_Effects11/lib/x86",
			"../Libraries/PhysX 3.4/lib/x86",
			"../Libraries/Nvidia Flex 1.1.0/lib/x86"
		}

		postbuildcommands 
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\fmod.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\x86\\cudart32_80.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\nvToolsExt32_1.dll\" \"$(OutDir)\" /y /D"
		}

		configuration "Debug32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\x86\\NvFlexDebugD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\x86\\NvFlexExtDebugD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PhysX3DEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PhysX3CommonDEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PxPvdSDKDEBUG_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PxFoundationDEBUG_x86.dll\" \"$(OutDir)\" /D"
		 	}

		configuration "Release32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\x86\\NvFlexReleaseD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.1.0\\bin\\x86\\NvFlexExtReleaseD3D_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PhysX3_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PhysX3Common_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PxPvdSDK_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\x86\\PxFoundation_x86.dll\" \"$(OutDir)\" /D"
		 	}


newaction {
		trigger     = "clean",
		description = "Remove all binaries and generated files",

		execute = function()
			os.rmdir("../Build")
		end
	}