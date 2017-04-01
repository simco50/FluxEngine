#!lua

solution "FluxEngine"
	basedir "../"
	configurations { "Debug32", "Release32" }
    architecture "x86"
    warnings "Extra"

   	configuration "Debug32"
   		runtime "Debug"
	 	defines { "_DEBUG" }
	 	flags {  }
	 	symbols "On"
	 	optimize "Off"
		targetdir "../Build/Engine/Debug32"

 	configuration "Release32"
		runtime "Release"
		defines { "NDEBUG" }
	 	flags {  }
	 	symbols "Off"
	 	optimize "Full"
	 	targetdir "../Build/Engine/Release32"

	project "FluxEngine"
		basedir "../FluxEngine/"
		location "../FluxEngine/"
		pchheader ("stdafx.h")
		pchsource ("../FluxEngine/stdafx.cpp")
		kind "ConsoleApp"
		language "C++"
		characterset ("MBCS")
		entrypoint ("")
		defines { "WIN32", "_CONSOLE" }

		files 
		{ 
			"../FluxEngine/**.h", 
			"../FluxEngine/**.hpp",
			"../FluxEngine/**.cpp" 
		}

		includedirs 
		{ 
			"../Libraries/ImgUI/include",
			"../Libraries/DX_Effects11/include",
			"../Libraries/DX_Tex/include",
			"../Libraries/Fmod/inc",
			"../Libraries/Nvidia Flex 1.0.0/include",
			"../Libraries/PhysX 3.4/Include"
		}

		libdirs
		{
			"../Libraries/Imgui/lib",
			"../Libraries/DX_Tex/lib/",
			"../Libraries/Fmod/lib",
			"../Libraries/DX_Effects11/lib",
			"../Libraries/PhysX 3.4/lib/vc14win32",
			"../Libraries/Nvidia Flex 1.0.0/lib/win32"
		}

		postbuildcommands 
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\fmod.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\Nvidia Flex 1.0.0\\bin\\win32\\cudart32_70.dll\" \"$(OutDir)\" /y /D",
			"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\nvToolsExt32_1.dll\" \"$(OutDir)\" /y /D"
		}

		configuration "Debug32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.0.0\\bin\\win32\\flexDebug_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.0.0\\bin\\win32\\flexExtDebug_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3DEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3CommonDEBUG_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxPvdSDKDEBUG_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxFoundationDEBUG_x86.dll\" \"$(OutDir)\" /D",
		 	}

		configuration "Release32"
		 	postbuildcommands
		 	{
		 		"copy \"..\\Libraries\\Nvidia Flex 1.0.0\\bin\\win32\\flexRelease_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\Nvidia Flex 1.0.0\\bin\\win32\\flexExtRelease_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PhysX3Common_x86.dll\" \"$(OutDir)\" /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxPvdSDK_x86.dll\" \"$(OutDir)\" /y /D",
				"copy \"..\\Libraries\\PhysX 3.4\\bin\\vc14win32\\PxFoundation_x86.dll\" \"$(OutDir)\" /D",
		 	}

solution "FluxConverter"
	basedir "../"
	configurations { "Debug32", "Release32" }
    architecture "x86"
    warnings "Extra"

    configuration "Debug32"
   		runtime "Debug"
	 	defines { "_DEBUG" }
	 	flags {  }
	 	symbols "On"
	 	optimize "Off"
		targetdir "../Build/Converter/Debug32"

 	configuration "Release32"
		runtime "Release"
		defines { "NDEBUG" }
	 	flags {  }
	 	symbols "Off"
	 	optimize "Full"
	 	targetdir "../Build/Converter/Release32"

	project "FluxConverter"
		basedir "../FluxConverter/"
		location "../FluxConverter/"
		pchheader ("stdafx.h")
		pchsource ("../FluxConverter/stdafx.cpp")
		kind "ConsoleApp"
		language "C++"
		characterset ("MBCS")
		entrypoint ("")
		defines { "WIN32", "_CONSOLE" }
		files 
		{ 
			"../FluxConverter/**.h", 
			"../FluxConverter/**.cpp" 
		}

		includedirs 
		{ 
			"../Libraries/Assimp/include"
		}

		libdirs 
		{
			"../Libraries/Assimp/lib/x86"
		}

		postbuildcommands 
		{ 
			"copy \"..\\Libraries\\Assimp\\bin\\x86\\Assimp32.dll\" \"$(OutDir)\" /y /D",
		}
		