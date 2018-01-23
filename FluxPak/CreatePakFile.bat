SET checkoutDir=%~1
"FluxPak/PakFileCompression.exe" -r "FluxPak/ResponseFile.txt" -p "FluxGame/Resources.pak" -v 0 -b "%checkoutDir%FluxGame/Resources/"