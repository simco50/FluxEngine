#include "stdafx.h"
#include "Core/FluxCore.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	FluxCore* core = new FluxCore();
	core->Run(hInstance);
	delete core;

	return 0;
}