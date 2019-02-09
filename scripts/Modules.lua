function AddModules(isTarget)

    if(isTarget == false) then
        removefiles
        {
            (ROOT .. engineName .. "/Rendering/Core/D3DCommon/**"),
            (ROOT .. engineName .. "/Rendering/Core/D3D11/**"),
            (ROOT .. engineName .. "/Rendering/Core/D3D12/**"),
            (ROOT .. engineName .. "/Physics/PhysX/**")
        }
    end

    AddPhysX(isTarget)
    AddFmod(isTarget)
    AddSQLite(isTarget)
    AddZlib(isTarget)
    AddSDL2(isTarget)
    AddAssimp(isTarget)
    if(_OPTIONS["gfxapi"] == "d3d11" or _OPTIONS["gfxapi"] == nil) then
        AddDX11(isTarget)
    else
        AddDX12(isTarget)
    end
    AddWininet(isTarget)
end

function AddPhysX(isTarget)
    defines { "MODULE_PHYSX" }

	includedirs 
	{ 
		(ROOT .. "Libraries/PhysX/PhysX/include"),
		(ROOT .. "Libraries/PhysX/PxShared/include")
	}
    
    if(isTarget == true) then
        filter { "platforms:x64" }
            links
            {
                "PhysX_64.lib",
                "PhysxFoundation_64.lib",
                "PhysXTask_static_64.lib",
                "PhysXExtensions_static_64.lib",
            }

        filter { "platforms:x86" }
            postbuildcommands
            {
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXCommon_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysX_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\%{cfg.buildcfg}\\PhysXFoundation_32.dll\" \"$(OutDir)\"",
            }
            links
            {
                "PhysX_32.lib",
                "PhysxFoundation_32.lib",
                "PhysXTask_static_32.lib",
                "PhysXExtensions_static_32.lib",
            }

        filter { "platforms:x86", "configurations:Debug" }
            links { "PhysXPvdSDK_static_32.lib" }

        filter { "platforms:x64", "configurations:Debug" }
            links { "PhysXPvdSDK_static_64.lib" }

        filter {"platforms:x64", "configurations:Release or Test"}
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysXCommon_64.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysX_64.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysXFoundation_64.dll\" \"$(OutDir)\"",
            }
            libdirs { "$(SolutionDir)Libraries/PhysX/PhysX/lib/%{cfg.platform}/Release" }

        filter {"platforms:x64", "configurations:Debug"}
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysXCommon_64.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysX_64.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysXFoundation_64.dll\" \"$(OutDir)\"",
            }
            libdirs { "$(SolutionDir)Libraries/PhysX/PhysX/lib/%{cfg.platform}/Debug" }

        filter {"platforms:x86", "configurations:Release or Test"}
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysXCommon_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysX_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Release\\PhysXFoundation_32.dll\" \"$(OutDir)\"",
            }
            libdirs { "$(SolutionDir)Libraries/PhysX/PhysX/lib/%{cfg.platform}/Release" }

        filter {"platforms:x86", "configurations:Debug"}
            postbuildcommands
            { 
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysXCommon_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysX_32.dll\" \"$(OutDir)\"",
                "{COPY} \"$(SolutionDir)Libraries\\PhysX\\PhysX\\bin\\%{cfg.platform}\\Debug\\PhysXFoundation_32.dll\" \"$(OutDir)\"",
            }
            libdirs { "$(SolutionDir)Libraries/PhysX/PhysX/lib/%{cfg.platform}/Debug" }

    else
        files 
        { 
            (ROOT .. engineName .. "/Physics/PhysX/**")
        }
        end
    filter {}
end

function AddFmod(isTarget)
    defines { "MODULE_FMOD" }
	includedirs (ROOT .. "Libraries/Fmod/inc")
    libdirs	(ROOT .. "Libraries/Fmod/lib/%{cfg.platform}")
    
    if(isTarget == true) then
        filter { "platforms:x64" }
            postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x64\\fmod64.dll\" \"$(OutDir)\"" }
            links "fmod64_vc.lib"
        filter { "platforms:x86" }
            postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\Fmod\\bin\\x86\\fmod.dll\" \"$(OutDir)\"" }
            links "fmod_vc.lib"
        end
    filter {}
end

function AddSQLite(isTarget)
    defines { "MODULE_SQLITE" }
	includedirs (ROOT .. "Libraries/SQLite/include")
    libdirs	(ROOT .. "Libraries/SQLite/lib/%{cfg.platform}")

    if(isTarget == true) then
        postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\SQLite\\bin\\%{cfg.platform}\\SQlite3.dll\" \"$(OutDir)\"" }
        links "sqlite3.lib"
        end
    filter {}
end

function AddZlib(isTarget)
    defines { "MODULE_ZLIB" }
	includedirs (ROOT .. "Libraries/Zlib/include")
    libdirs	(ROOT .. "Libraries/Zlib/lib/%{cfg.platform}")

    if(isTarget == true) then
        filter { "configurations:Debug" }
            postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib_DEBUG.dll\" \"$(OutDir)\"" }
        filter { "configurations:Release or Test" }
            postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\Zlib\\bin\\%{cfg.platform}\\Zlib.dll\" \"$(OutDir)\"" }
        filter { "configurations:Debug" }
            links "zlib_DEBUG.lib"
        filter { "configurations:Release or Test" }
            links "zlib.lib" 
        end
    filter {}
end

function AddSDL2(isTarget)
    defines { "MODULE_SDL2" }
	includedirs (ROOT .. "Libraries/SDL2-2.0.7/include")
    libdirs	(ROOT .. "Libraries/SDL2-2.0.7/lib/%{cfg.platform}")
    
    if(isTarget == true) then
        postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\SDL2-2.0.7\\lib\\%{cfg.platform}\\SDL2.dll\" \"$(OutDir)\"" }
        links
        {
            "sdl2.lib",
            "sdl2main.lib"
        }
        end
end

function AddAssimp(isTarget)
    defines { "MODULE_ASSIMP" }
	includedirs (ROOT .. "Libraries/Assimp/include")
    libdirs	(ROOT .. "Libraries/Assimp/lib/%{cfg.platform}")
    
    if(isTarget == true) then
        postbuildcommands { "{COPY} \"$(SolutionDir)Libraries\\Assimp\\bin\\%{cfg.platform}\\assimp-vc140-mt.dll\" \"$(OutDir)\"" }
        links "assimp-vc140-mt.lib"
    end
end

function AddDX11(isTarget)
    defines { "MODULE_D3D11", "GRAPHICS_D3D11", "GRAPHICS_D3D" }

    if(isTarget == true) then
        links
        {
            "dxgi.lib",
            "d3d11.lib",
            "d3dcompiler.lib",
            "dxguid.lib",
        }
    else
        files 
        { 
            (ROOT .. engineName .. "/Rendering/Core/D3DCommon/**"),
            (ROOT .. engineName .. "/Rendering/Core/D3D11/**") 
        }
    end
end

function AddDX12(isTarget)
    defines { "MODULE_D3D12", "GRAPHICS_D3D12", "GRAPHICS_D3D" }

    if(isTarget == true) then
        links
        {
            "dxgi.lib",
            "d3d12.lib",
            "d3dcompiler.lib",
            "dxguid.lib",
        }
    else
        files 
        { 
            (ROOT .. engineName .. "/Rendering/Core/D3DCommon/**"),
            (ROOT .. engineName .. "/Rendering/Core/D3D12/**")
        }
    end
end

function AddWininet(isTarget)
    defines { "MODULE_WININET" }

    if(isTarget == true) then
        links { "wininet.lib" }
    end
end