#include "FluxGame.h"
#include "Core/Application.h"
#include "Core/Context.h"
#include "Core/CommandLine.h"

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nShowCmd*/)
{
	
#ifdef _DEBUG
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(4322);
#endif
	CommandLine::Parse(lpCmdLine);

	Application* pCore = new Application();
	pCore->Run();
	delete pCore;
	return 0;
}