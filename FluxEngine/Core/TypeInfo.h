#pragma once

using StringHash = size_t;

class TypeInfo
{
public:

	/// Construct.
	TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
	/// Destruct.
	~TypeInfo();

	/// Check current type is type of specified type.
	bool IsTypeOf(const StringHash& type) const;
	/// Check current type is type of specified type.
	bool IsTypeOf(const TypeInfo* typeInfo) const;

	/// Check current type is type of specified class type.
	template<typename T> bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

	/// Return type.
	const StringHash& GetType() const { return m_Type; }
	/// Return type name.
	const std::string& GetTypeName() const { return m_TypeName; }
	/// Return base type info.
	const TypeInfo* GetBaseTypeInfo() const { return m_pBaseTypeInfo; }

private:
	std::string m_TypeName;
	StringHash m_Type;
	/// Base class type info.
	const TypeInfo* m_pBaseTypeInfo = nullptr;
};
