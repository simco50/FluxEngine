#include "stdafx.h"

#include "Game/GameManager.h"


int main()
{
	wWinMain(GetModuleHandle(nullptr), nullptr, nullptr, SW_SHOW);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nShowCmd)
{
	UNREFERENCED_PARAMETER(pCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);
	UNREFERENCED_PARAMETER(hPrevInstance);

	GameManager::Instance().Run(hInstance);

	return 0;
}