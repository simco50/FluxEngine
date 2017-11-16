echo off
cls
D:\Software\CppCheck\cppcheck.exe --enable=style -iExternal "FluxEngine" --force --suppress=noExplicitConstructor cstyleCast  2> cppCheckResults.txt
pause