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
	Object() {}
	virtual ~Object() {}

	virtual StringHash GetType() const { return GetTypeInfoStatic()->GetType(); } \
	virtual const std::string& GetTypeName() const { return GetTypeInfoStatic()->GetTypeName(); } \
	virtual const TypeInfo* GetTypeInfo() const { return GetTypeInfoStatic(); } \
	static StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
	static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
	static const TypeInfo* GetTypeInfoStatic() { static const TypeInfo typeInfoStatic("Object", nullptr); return &typeInfoStatic; } \

private:

};