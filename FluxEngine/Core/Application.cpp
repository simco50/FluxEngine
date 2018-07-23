#include "FluxEngine.h"
#include "Application.h"
#include "FluxCore.h"

Application::Application() :
	Object(new Context())
{
	m_pEngine = new FluxCore(m_pContext);
}

Application::~Application()
{
	delete m_pContext;
	delete m_pEngine;
}

int Application::Run()
{
	int exitCode = m_pEngine->Run(nullptr);
	if (exitCode > 0)
	{
		return exitCode;
	}

	Start();

	while (FluxCore::IsExiting() == false)
	{
		m_pEngine->ProcessFrame();
	}

	Stop();

	return exitCode;
}