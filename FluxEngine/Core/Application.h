#pragma once

class FluxCore;

class Application
{
public:
	Application();
	virtual ~Application();

	int Run();
	virtual void Start() {}
	virtual void Stop() {}

private:
	Context* m_pContext;
	FluxCore* m_pEngine = nullptr;
};