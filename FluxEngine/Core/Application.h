#pragma once

class FluxCore;

class Application
{
	DELETE_COPY(Application)

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