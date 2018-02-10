#pragma once
#include "Object.h"

class Subsystem : public Object
{
	FLUX_OBJECT(Subsystem, Object)

public:
	Subsystem(Context* pContext);
	virtual ~Subsystem();

private:
};