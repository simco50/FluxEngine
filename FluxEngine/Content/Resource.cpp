#include "FluxEngine.h"
#include "Resource.h"

Resource::Resource(Context* pContext)
	: Object(pContext)
{

}

Resource::~Resource()
{

}

bool Resource::Save(OutputStream& /*outputStream*/)
{
	return false;
}
