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
end

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