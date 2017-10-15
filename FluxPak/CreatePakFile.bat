SET checkoutDir=%~1
"%~dp0/FluxPak/PakFileCompression.exe" -r "FluxPak/ResponseFile.txt" -p "%~dp0/FluxGame/Resources.pak" -b "%checkoutDir%FluxGame/Resources/"