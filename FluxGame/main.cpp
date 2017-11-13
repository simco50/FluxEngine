#include "stdafx.h"
#include <Core/FluxCore.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
	// Enable run-time memory leak check for debug builds.
#ifdef _DEBUG
	// notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);

	//_CrtSetBreakAlloc(4322);
#endif

	unique_ptr<FluxCore> pCore = make_unique<FluxCore>();
	pCore->Run(hInstance);

	return 0;
}