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
	return NewObject(StringHash(typeName));
}

Object* Object::NewObject(StringHash type) const
{
	const TypeInfo* pType = m_pContext->GetTypeInfo(type);
	return NewObject(pType);
}

Object* Object::NewObject(const TypeInfo* pType) const
{
	if (pType == nullptr)
	{
		FLUX_LOG(Warning, "[Object::NewObject] Type is not registed");
		return nullptr;
	}
	checkf(pType->IsAbstract() == false, "[Object::NewObject] Can't create instance of an abstract class");
	return pType->CreateInstance(m_pContext);
}
