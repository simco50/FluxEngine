echo off
cls

echo Building for Visual Studio 2017...
premake5 vs2017
echo Building for Visual Studio 2015...
premake5 vs2015
pause