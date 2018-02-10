#include "FluxEngine.h"
#include "Resource.h"

Resource::Resource(Context* pContext) :
	Object(pContext)
{

}

Resource::~Resource()
{

}

bool Resource::Save(const std::string& filePath)
{
	UNREFERENCED_PARAMETER(filePath);
	return false;
}
