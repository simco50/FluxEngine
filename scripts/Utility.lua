function SetupTarget(name, applicationType)
project (name)
	filename (name)
	location (ROOT .. name)
	targetdir (ROOT .. "Build/" .. name .. "_$(Platform)_$(Configuration)")
	objdir ("!" .. ROOT .. "Build/Intermediate/$(ProjectName)_$(Platform)_$(Configuration)")

	pchheader (name .. ".h")
	pchsource (ROOT .. name .. "/" .. name .. ".cpp")
	kind (applicationType)
end

function AddSourceFiles(directoryPath)
    files
    { 
        ("../" .. directoryPath .. "/**.h"),
        ("../" .. directoryPath .. "/**.hpp"),
        ("../" .. directoryPath .. "/**.cpp"),
        ("../" .. directoryPath .. "/**.inl"),
        ("../" .. directoryPath .. "/**.c"),
        ("../" .. directoryPath .. "/**.natvis"),
	}
	
	removefiles
	{
		(ROOT .. engineName .. "/Rendering/Core/D3DCommon/**"),
		(ROOT .. engineName .. "/Rendering/Core/D3D11/**"),
		(ROOT .. engineName .. "/Rendering/Core/D3D12/**")
	}

end

newaction {
	trigger     = "clean",
	description = "Remove all binaries and generated files",

	execute = function()
		os.rmdir("../Build")
		os.rmdir("../ipch")
		os.rmdir("../.vs")
		os.rmdir("../FluxGame/Saved")
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