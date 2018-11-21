function AddPhysX(isTarget)

    defines { "MODULE_PHYSX" }

	includedirs 
	{ 
		"../Libraries/PhysX 3.4/PhysX_3.4/include",
		"../Libraries/PhysX 3.4/PxShared/include"
	}

	libdirs
	{
		"../Libraries/PhysX 3.4/PhysX_3.4/lib/%{cfg.platform}",
		"../Libraries/PhysX 3.4/PxShared/lib/%{cfg.platform}"
    }
    
    if(isTarget == true) then

        filter { "configurations:Debug" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\%{cfg.platform}\\PhysX3CommonDEBUG_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\%{cfg.platform}\\PhysX3DEBUG_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\%{cfg.platform}\\PxFoundationDEBUG_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\%{cfg.platform}\\PxPvdSDKDEBUG_%{cfg.platform}.dll\" \"$(OutDir)\"",
            }

        filter { "configurations:Release or Test" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\%{cfg.platform}\\PhysX3Common_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\%{cfg.platform}\\PhysX3_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\%{cfg.platform}\\PxFoundation_%{cfg.platform}.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PxShared\\bin\\%{cfg.platform}\\PxPvdSDK_%{cfg.platform}.dll\" \"$(OutDir)\"",
            }

        filter { "platforms:x64" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x64\\nvToolsExt64_1.dll\" \"$(OutDir)\""
            }

        filter { "platforms:x86" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX 3.4\\PhysX_3.4\\bin\\x86\\nvToolsExt32_1.dll\" \"$(OutDir)\""
            }

        filter { "configurations:Debug" }
            links
            {
                "PhysX3DEBUG_%{cfg.platform}.lib",
                "PhysX3ExtensionsDEBUG.lib",
                "PxFoundationDEBUG_%{cfg.platform}.lib",
                "PxTaskDEBUG_%{cfg.platform}.lib",
                "PxPvdSDKDEBUG_%{cfg.platform}.lib",
            }
        filter { "configurations:Release or Test" }
            links
            {
                "PhysX3_%{cfg.platform}.lib",
                "PhysX3Extensions.lib",
                "PxFoundation_%{cfg.platform}.lib",
                "PxTask_%{cfg.platform}.lib",
                "PxPvdSDK_%{cfg.platform}.lib",
            }
        end
    filter {}
end

function AddFmod(isTarget)

    defines { "MODULE_FMOD" }

	includedirs "../Libraries/Fmod/inc"

    libdirs	"../Libraries/Fmod/lib/%{cfg.platform}"
    
    if(isTarget == true) then
        --filter { "configurations:Debug", "platforms:x64" }
        --    links "fmodL64_vc.lib"
        --    postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmodL64.dll\" \"$(OutDir)\""
        --filter { "configurations:Release or Test", "platforms:x64" }
        --    postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\""
        --    links "fmod64_vc.lib"
        --filter { "configurations:Debug", "platforms:x86" }
        --    postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmodL.dll\" \"$(OutDir)\""
        --    links "fmodL_vc.lib"
        --filter { "configurations:Release or Test", "platforms:x86" }
        --    postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\""
        --    links "fmod_vc.lib"

        --Don't want the debug libs in Debug
        filter { "platforms:x64" }
            postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\""
            links "fmod64_vc.lib"
        filter { "platforms:x86" }
            postbuildcommands "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\""
            links "fmod_vc.lib"
        end
    filter {}
end

function AddSQLite(isTarget)
    
    defines { "MODULE_SQLITE" }

	includedirs "../Libraries/SQLite/include"

    libdirs	"../Libraries/SQLite/lib/%{cfg.platform}"

    if(isTarget == true) then
        postbuildcommands
        { 
            "{COPY} \"$(SolutionDir)Libraries\\SQLite\\bin\\%{cfg.platform}\\SQlite3.dll\" \"$(OutDir)\""
        }
        links "sqlite3.lib"
        end
    filter {}
end

function AddZlib(isTarget)
    
    defines { "MODULE_ZLIB" }

	includedirs "../Libraries/Zlib/include"

    libdirs	"../Libraries/Zlib/lib/%{cfg.platform}"

    if(isTarget == true) then
        filter { "configurations:Debug" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib_DEBUG.dll\" \"$(OutDir)\""
            }

        filter { "configurations:Release or Test" }
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib.dll\" \"$(OutDir)\""
            }

        filter { "configurations:Debug" }
            links "zlib_DEBUG.lib"
        filter { "configurations:Release or Test" }
            links "zlib.lib" 

        end
    filter {}
end

function AddSDL2(isTarget)
    
    defines { "MODULE_SDL2" }

	includedirs "../Libraries/SDL2-2.0.7/include"

    libdirs	"../Libraries/SDL2-2.0.7/lib/%{cfg.platform}"
    
    if(isTarget == true) then
        postbuildcommands
        { 
            "{COPY} \"$(SolutionDir)Libraries\\SDL2-2.0.7\\lib\\%{cfg.platform}\\SDL2.dll\" \"$(OutDir)\""
        }
        links
        {
            "sdl2.lib",
            "sdl2main.lib"
        }
        end
end

function AddAssimp(isTarget)
    
    defines { "MODULE_ASSIMP" }

	includedirs "../Libraries/Assimp/include"

    libdirs	"../Libraries/Assimp/lib/%{cfg.platform}"
    
    if(isTarget == true) then
        postbuildcommands
        { 
            "{COPY} \"$(SolutionDir)Libraries\\Assimp\\bin\\%{cfg.platform}\\assimp-vc140-mt.dll\" \"$(OutDir)\""
        }
        links "assimp-vc140-mt.lib"
    end
end

function AddDX11(isTarget)

    defines { "MODULE_D3D11", "GRAPHICS_D3D11" }

    if(isTarget == true) then
        links
        {
            "dxgi.lib",
            "d3d11.lib",
            "d3dcompiler.lib",
            "dxguid.lib",
        }
    end
end

function AddWininet(isTarget)
    defines { "MODULE_WININET" }

    if(isTarget == true) then
        links "wininet.lib"
    end
end