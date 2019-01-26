#pragma once

class Context;
class Subsystem;

#define FLUX_OBJECT(typeName, baseTypeName) \
    public: \
        using ClassName = typeName; \
        using BaseClass = baseTypeName; \
        virtual StringHash GetType() const override { return GetTypeInfoStatic()->GetType(); } \
        virtual const char* GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
        static constexpr StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static constexpr const char* GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
		static constexpr const TypeInfo* GetTypeInfoStatic() { return &TYPE_INFO; } \
	private: \
		static constexpr TypeInfo TYPE_INFO = TypeInfo(#typeName, baseTypeName::GetTypeInfoStatic()); \

class Object
{
public:
	virtual ~Object()
	{}

	virtual StringHash GetType() const { return GetTypeInfoStatic()->GetType(); }
	virtual const char* GetTypeName() const { return GetTypeInfoStatic()->GetTypeName(); }
	virtual const TypeInfo* GetTypeInfo() const { return GetTypeInfoStatic(); }
	static constexpr StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); }
	static constexpr const char* GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); }
	static constexpr const TypeInfo* GetTypeInfoStatic() { return &TYPE_INFO; }

	bool IsTypeOf(const TypeInfo* pTypeInfo) const;
	bool IsTypeOf(StringHash type) const;
	template<typename T>
	inline bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

	Subsystem* GetSubsystem(StringHash type) const;
	template<typename T>
	inline T* GetSubsystem(bool required = true) const 
	{ 
		return static_cast<T*>(m_pContext->GetSubsystem(T::GetTypeStatic(), required));
	}
	inline Context* GetContext() const 
	{
		return m_pContext; 
	}

	Object* NewObject(const char* typeName) const;
	Object* NewObject(StringHash type) const;

	template<typename T>
	T* NewObject(StringHash type = T::GetTypeStatic()) const
	{
		Object* pObject = NewObject(type);
		if (pObject->IsTypeOf(T::GetTypeStatic()))
		{
			return static_cast<T*>(pObject);
		}
		delete pObject;
		return nullptr;
	}

protected:
	explicit Object(Context* pContext)
		: m_pContext(pContext)
	{}

	Context* m_pContext = nullptr;

private:
	static constexpr TypeInfo TYPE_INFO = TypeInfo("Object", nullptr);
};

template<typename T>
T* DynamicCast(Object* pObject)
{
	if (pObject && pObject->IsTypeOf(T::GetTypeStatic()))
	{
		return static_cast<T*>(pObject);
	}
	return nullptr;
}

template<typename T>
std::shared_ptr<T> DynamicCastPtr(const std::shared_ptr<Object>& pObject)
{
	if (pObject->IsTypeOf(T::GetTypeStatic()))
	{
		return std::static_pointer_cast<T>(pObject);
	}
	return std::shared_ptr<T>();
}