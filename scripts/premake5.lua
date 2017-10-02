solution ("FluxEngine")
	basedir "../"
	configurations { "Debug", "Release" }
    platforms "x64"
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
		}

		libdirs
		{
			"../Libraries/Imgui/lib/x64",
			"../Libraries/DX_Tex/lib/x64",
			"../Libraries/Fmod/lib/x64",
			"../Libraries/DX_Effects11/lib/x64",
		}

		postbuildcommands 
		{ 
			"copy \"..\\Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\" /y /D",
		}


newaction {
		trigger     = "clean",
		description = "Remove all binaries and generated files",

		execute = function()
			os.rmdir("../Build")
		end
	}