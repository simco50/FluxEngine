#pragma once

class TypeInfo
{
public:
	template<size_t size>
	constexpr TypeInfo(const char (&typeName)[size], const TypeInfo* baseTypeInfo) 
		: m_TypeName{},
		m_Type(typeName),
		m_pBaseTypeInfo(baseTypeInfo)
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

private:
	static constexpr size_t TYPENAME_SIZE = 32;
	char m_TypeName[TYPENAME_SIZE];
	StringHash m_Type;
	const TypeInfo* m_pBaseTypeInfo = nullptr;
};