#include "FluxEngine.h"
#include "TypeInfo.h"

bool TypeInfo::IsTypeOf(const TypeInfo* typeInfo) const
{
	check(typeInfo);
	return IsTypeOf(typeInfo->GetType());
}

bool TypeInfo::IsTypeOf(const StringHash& type) const
{
	const TypeInfo* pInfo = this;
	while (pInfo != nullptr)
	{
		if (type == pInfo->m_Type)
		{
			return true;
		}
		pInfo = pInfo->m_pBaseTypeInfo;
	}
	return false;
}
