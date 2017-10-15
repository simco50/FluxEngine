SET checkoutDir=%~1
"%~dp0/PakFileCompression.exe" -r "ResponseFile.txt" -p "%~dp0/../FluxGame/Resources.pak" -b "%checkoutDir%FluxGame/Resources/"