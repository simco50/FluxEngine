#include "FluxGame.h"
#include "Core/Application.h"
#include "Core/CommandLine.h"
#include "Helpers/Compression.h"

int WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR lpCmdLine, _In_ int /*nShowCmd*/)
{
#ifdef _DEBUG
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(2103);
#endif
	CommandLine::Parse(lpCmdLine);

	Application* pCore = new Application();
	pCore->Run();
	delete pCore;
	return 0;
}