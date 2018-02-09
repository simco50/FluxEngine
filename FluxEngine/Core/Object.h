#pragma once

class Context;

#define FLUX_OBJECT(typeName, baseTypeName) \
    public: \
        using ClassName = typeName; \
        using BaseClassName = baseTypeName; \
        virtual StringHash GetType() const override { return GetTypeInfoStatic()->GetType(); } \
        virtual const std::string& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
        static StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
        static const TypeInfo* GetTypeInfoStatic() { static const TypeInfo typeInfoStatic(#typeName, baseTypeName::GetTypeInfoStatic()); return &typeInfoStatic; } \

class Object
{
public:
	Object(Context* pContext) : m_pContext(pContext) {}
	virtual ~Object() {}

	virtual StringHash GetType() const { return GetTypeInfoStatic()->GetType(); }
	virtual const std::string& GetTypeName() const { return GetTypeInfoStatic()->GetTypeName(); }
	virtual const TypeInfo* GetTypeInfo() const { return GetTypeInfoStatic(); }
	static StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); }
	static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); }
	static const TypeInfo* GetTypeInfoStatic() { static const TypeInfo typeInfoStatic("Object", nullptr); return &typeInfoStatic; }

	bool IsTypeOf(const TypeInfo* pTypeInfo) { return GetTypeInfo()->IsTypeOf(pTypeInfo); }
	bool IsTypeOf(StringHash type) { return GetTypeInfo()->IsTypeOf(type); }
	template<typename T>
	bool IsTypeOf() { return IsTypeOf(T::GetTypeInfoStatic()); }

	void SetContext(Context* pContext) { m_pContext = pContext; }
	Context* GetContext() const { return m_pContext; }

protected:
	Context* m_pContext = nullptr;

private:

};

template<typename T>
T* DynamicCast(Object* pObject)
{
	if (pObject->GetTypeInfo()->IsTypeOf<T>())
		return static_cast<T*>(pObject);
	return nullptr;
}