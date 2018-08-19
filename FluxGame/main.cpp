#include "FluxGame.h"
#include "Core/Application.h"
#include "Core/Context.h"
#include "Core/CommandLine.h"

int WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR lpCmdLine, _In_ int /*nShowCmd*/)
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