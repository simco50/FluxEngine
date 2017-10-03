solution ("FluxEngine")
	basedir "../"
	configurations { "Debug", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"

	filter { "platforms:x64" }
		architecture "x64"
		postbuildcommands
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\" /y /D",
		}
		defines {"x64"}

	filter { "platforms:x86" }
		architecture "x32"
		postbuildcommands
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\" /y /D",
		}
		defines {"x86"}

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
			"../Libraries/DX_Effects11/include",
			"../Libraries/DX_Tex/include",
			"../Libraries/Fmod/inc",
		}

		libdirs
		{
			"../Libraries/Imgui/lib/%{cfg.platform}",
			"../Libraries/DX_Tex/lib/%{cfg.platform}",
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/DX_Effects11/lib/%{cfg.platform}",
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

newaction {
		trigger     = "clean",
		description = "Remove all binaries and generated files",

		execute = function()
			os.rmdir("../Build")
			os.rmdir("../ipch")
		end
	}