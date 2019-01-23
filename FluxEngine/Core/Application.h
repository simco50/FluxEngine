#pragma once

#include "Object.h"

class FluxCore;

class Application : public Object
{
	FLUX_OBJECT(Application, Object)
	DELETE_COPY(Application)

public:
	Application();
	virtual ~Application();

	int Run();
	virtual void Start() {}
	virtual void Stop() {}

private:
	FluxCore* m_pEngine = nullptr;
};