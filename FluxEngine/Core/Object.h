#pragma once

class Context;
class Subsystem;

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
	private: \
		virtual void NOTYPEINFO() {} \

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

	bool IsTypeOf(const TypeInfo* pTypeInfo);
	bool IsTypeOf(StringHash type);
	template<typename T>
	inline bool IsTypeOf() { return IsTypeOf(T::GetTypeInfoStatic()); }

	Subsystem* GetSubsystem(StringHash type) const;
	template<typename T>
	T* GetSubsystem(bool required = true) const { return m_pContext->GetSubsystem<T>(required); }

protected:
	//This enforces the FLUX_OBJECT on all classes that inherit from Object
	virtual void NOTYPEINFO() = 0;

	Context* m_pContext = nullptr;
};

template<typename T>
T* DynamicCast(Object* pObject)
{
	if (pObject->GetTypeInfo()->IsTypeOf<T>())
		return static_cast<T*>(pObject);
	return nullptr;
}