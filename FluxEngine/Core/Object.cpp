#include "FluxEngine.h"
#include "Object.h"

bool Object::IsTypeOf(const TypeInfo* pTypeInfo)
{
	return GetTypeInfo()->IsTypeOf(pTypeInfo);
}

bool Object::IsTypeOf(StringHash type)
{
	return GetTypeInfo()->IsTypeOf(type);
}

Subsystem* Object::GetSubsystem(StringHash type) const
{
	return m_pContext->GetSubsystem(type);
}
