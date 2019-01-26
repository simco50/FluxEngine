#include "FluxEngine.h"
#include "Object.h"

bool Object::IsTypeOf(const TypeInfo* pTypeInfo) const
{
	return GetTypeInfo()->IsTypeOf(pTypeInfo);
}

bool Object::IsTypeOf(StringHash type) const
{
	return GetTypeInfo()->IsTypeOf(type);
}

Subsystem* Object::GetSubsystem(StringHash type) const
{
	return m_pContext->GetSubsystem(type);
}

Object* Object::NewObject(const char* typeName) const
{
	return m_pContext->NewObject(StringHash(typeName));
}

Object* Object::NewObject(StringHash type) const
{
	return m_pContext->NewObject(type);
}