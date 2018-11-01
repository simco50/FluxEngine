workspace "FluxEngine"
	filename "FluxEngine"
	basedir "../"
	configurations { "Debug", "Release" }
    platforms {"x86", "x64"}
    warnings "Extra"
    --rtti "Off"
	characterset ("UNICODE")
	defines { "_CONSOLE" }
	--flags {"FatalWarnings"}
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

		pchheader "stdafx.h"
		pchsource "../FluxEngine/Core/stdafx.cpp"
		kind "ConsoleApp"

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
			"../Libraries/DX_Effects11/include",
			"../Libraries/DX_Tex/include",
			"../Libraries/Nvidia Flex 1.0.0/include",
		}

		libdirs
		{
			"../Libraries/Fmod/lib/%{cfg.platform}",
			"../Libraries/DX_Effects11/lib/%{cfg.platform}",
			"../Libraries/DX_Tex/lib/%{cfg.platform}",
			"../Libraries/Nvidia Flex 1.0.0/lib/%{cfg.platform}",
		}

		filter "files:**/External/**.*"
			flags "NoPCH"

		filter { "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x64\\cudart64_70.dll\" \"$(OutDir)\"",
			}

		filter { "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x86\\cudart32_70.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Debug", "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x86\\flexDebug_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x86\\flexExtDebug_x86.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Release", "platforms:x86" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x86\\flexRelease_x86.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x86\\flexExtRelease_x86.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Debug", "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x64\\flexDebug_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x64\\flexExtDebug_x64.dll\" \"$(OutDir)\"",
			}

		filter { "configurations:Release", "platforms:x64" }
			postbuildcommands
			{ 
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x64\\flexRelease_x64.dll\" \"$(OutDir)\"",
				"{COPY} \"$(SolutionDir)Libraries\\Nvidia Flex 1.0.0\\bin\\x64\\flexExtRelease_x64.dll\" \"$(OutDir)\"",
			}
			
newaction {
	trigger     = "clean",
	description = "Remove all binaries and generated files",

	execute = function()
		os.rmdir("../Build")
		os.rmdir("../ipch")
		os.remove("../.vs")
		os.remove("../*.sln")
		os.remove("../FluxEngine/*.vcxproj.*")
	end
}