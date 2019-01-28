#pragma once

class Context;
class Object;

class TypeInfo
{
public:
	using ObjectCreateFunction = Object * (*)(Context*);

	template<size_t size>
	constexpr TypeInfo(const char (&typeName)[size], const TypeInfo* baseTypeInfo, ObjectCreateFunction createFunction, const bool isAbstract) 
		: m_pBaseTypeInfo(baseTypeInfo),
		m_CreateFunction(createFunction),
		m_Type(typeName),
		m_IsAbstract(isAbstract),
		m_TypeName {}
	{
		static_assert(size <= TYPENAME_SIZE, "Size of typename is too long");
		constexpr size_t min = Math::Min<size_t>(size, TYPENAME_SIZE);
		for (size_t i = 0; i < min; ++i)
		{
			m_TypeName[i] = typeName[i];
		}
	}

	bool IsTypeOf(const StringHash& type) const;
	bool IsTypeOf(const TypeInfo* typeInfo) const;

	template<typename T>
	bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

	constexpr StringHash GetType() const { return m_Type; }
	constexpr const char* GetTypeName() const { return m_TypeName; }
	constexpr const TypeInfo* GetBaseTypeInfo() const { return m_pBaseTypeInfo; }
	constexpr bool IsAbstract() const { return m_IsAbstract; }
	Object* CreateInstance(Context* pContext) const { return m_CreateFunction(pContext); }

private:
	static constexpr size_t TYPENAME_SIZE = 32;
	const TypeInfo* m_pBaseTypeInfo = nullptr;
	ObjectCreateFunction m_CreateFunction;
	StringHash m_Type;
	bool m_IsAbstract;
	char m_TypeName[TYPENAME_SIZE];
};