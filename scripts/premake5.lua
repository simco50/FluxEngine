workspace "FluxEngine"
	filename "FluxEngine"
	basedir "../"
	configurations { "Debug", "Test", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"
    rtti "Off"
	characterset ("MBCS")
	defines { "_CONSOLE", "PHYSX", "THREADING", "GRAPHICS_D3D11" }
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
	 	defines { "FLUX_DEBUG", "_DEBUG", "PROFILING" }
	 	flags {  }
	 	symbols "On"
	 	optimize "Off"

 	filter { "configurations:Test" }
		runtime "Release"
	 	defines { "FLUX_TEST", "NDEBUG", "PROFILING" }
	 	flags {  }
	 	symbols "Off"
	 	optimize "Full"

	filter { "configurations:Release" }
	 	runtime "Release"
		defines { "FLUX_RELEASE", "NDEBUG" }
	 	flags {  }
	 	symbols "Off"
	 	optimize "Full"

	project "FluxEngine"
		filename "FluxEngine"
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
			"../Libraries/Fmod/inc",
			"../Libraries/Zlib/include",
			"../Libraries/PhysX 3.4/PhysX_3.4/include",
			"../Libraries/PhysX 3.4/PxShared/include",
			"../Libraries/SDL2-2.0.7/include",
			"../Libraries/Assimp/include",
		}

		libdirs
		{
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PhysX_3.4/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PxShared/lib/%{cfg.platform}",
			"../Libraries/SDL2-2.0.7/lib/%{cfg.platform}",
			"../Libraries/Assimp/lib/%{cfg.platform}",
		}

		filter "files:**/External/**.*"
			flags "NoPCH"

	project "FluxGame"
		filename "FluxGame"
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
			"../Libraries/Fmod/inc",
			"../Libraries/Zlib/include",
			"../Libraries/PhysX 3.4/PhysX_3.4/include",
			"../Libraries/PhysX 3.4/PxShared/include",
			"../Libraries/SDL2-2.0.7/include",
			"../Libraries/Assimp/include",
		}

		libdirs
		{
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/Zlib/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PhysX_3.4/lib/%{cfg.platform}",
			"../Libraries/PhysX 3.4/PxShared/lib/%{cfg.platform}",
			"../Libraries/SDL2-2.0.7/lib/%{cfg.platform}",
			"../Libraries/Assimp/lib/%{cfg.platform}",
		}

		postbuildcommands
		{ 
			"{COPY} \"$(SolutionDir)Libraries\\SDL2-2.0.7\\lib\\%{cfg.platform}\\SDL2.dll\" \"$(OutDir)\"",
			"{COPY} \"$(SolutionDir)Libraries\\Assimp\\bin\\%{cfg.platform}\\assimp-vc140-mt.dll\" \"$(OutDir)\"",
		}

		filter { "configurations:Debug" }
		 	postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib_DEBUG.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Release or Test" }
		 	postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib.dll\" \"$(OutDir)\"",
				"\"$(SolutionDir)Scripts\\premake5\" --file=\"$(SolutionDir)Scripts\\premake5.lua\" pak"
			}

		filter { "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\nvToolsExt64_1.dll\" \"$(OutDir)\"",
			}

		filter { "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\nvToolsExt32_1.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Debug", "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3CommonDEBUG_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3DEBUG_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxFoundationDEBUG_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxPvdSDKDEBUG_x86.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Release or Test", "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3Common_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\PhysX3_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxFoundation_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x86\\PxPvdSDK_x86.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Debug", "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3CommonDEBUG_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3DEBUG_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxFoundationDEBUG_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxPvdSDKDEBUG_x64.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Release or Test", "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3Common_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\PhysX3_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxFoundation_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\x64\\PxPvdSDK_x64.dll\" \"$(OutDir)\"",
			}
			
newaction {
	trigger     = "clean",
	description = "Remove all binaries and generated files",

	execute = function()
		os.rmdir("../Build")
		os.rmdir("../ipch")
		os.remove("../.vs")
		os.remove("../*.sln")
		os.remove("../FluxGame/*.vcxproj.*")
		os.remove("../FluxEngine/*.vcxproj.*")
	end
}

newaction {
	trigger 	= "pak",
	description	= "Creates a pak file",

	execute = function()
		os.execute("dir \"..\\FluxGame\\Resources\\\" /s /b /a-d > \"..\\FluxPak\\ResponseFile.txt\"")
		filePath = os.realpath("..\\FluxGame\\Resources")
		os.execute("..\\FluxPak\\PakFileCompression.exe -r \"../FluxPak/ResponseFile.txt\" -p \"../FluxGame/Resources.pak\" -b \"" .. filePath .. "/\"")
	end
}