#pragma once
#include "Object.h"
class Subsystem;

class Context
{
public:
	Context();
	~Context();

	void InitSDLSystem(unsigned int flag);
	void ShutdownSDL();

	Subsystem* GetSubsystem(StringHash type, bool required = true) const;

	template<typename T>
	T* GetSubsystem(bool required = true) const
	{
		return static_cast<T*>(GetSubsystem(T::GetTypeStatic(), required));
	}

	template<typename T>
	T* RegisterSubsystem()
	{
		return static_cast<T*>(RegisterSubsystem(T::GetTypeStatic(), [](Context* pContext) { return static_cast<Subsystem*>(new T(pContext)); }));
	}

	template<typename T>
	void RegisterFactory()
	{
		m_TypeTrees[T::GetTypeInfoStatic()->GetBaseTypeInfo()->GetType()].insert(T::GetTypeInfoStatic());
		m_Factories[T::GetTypeStatic()] = T::GetTypeInfoStatic();
	}

	std::vector<const TypeInfo*> GetAllTypesOf(StringHash type, bool includeAbstract = true);

	Object* NewObject(const StringHash typeHash, bool assertOnFailure = false);

private:
	void GetAllTypesOf(StringHash type, std::vector<const TypeInfo*>& typeData, bool includeAbstract);

	using SubsystemCreateFunction = Subsystem * (*)(Context*);

	Subsystem* RegisterSubsystem(StringHash typeHash, SubsystemCreateFunction createFunction)
	{
		auto pIt = m_Systems.find(typeHash);
		if (pIt != m_Systems.end())
		{
			FLUX_LOG(Warning, "[Content::RegisterSubsystem] > A subsystem with type '%d' is already registered", typeHash.m_Hash);
			return pIt->second;
		}
		Subsystem* pSystem = createFunction(this);
		m_SystemCache.push_back(pSystem);
		m_Systems[typeHash] = pSystem;
		return pSystem;
	}

	//A collection of subsystems
	std::unordered_map<StringHash, Subsystem*> m_Systems;
	//Vector to keep order of destruction
	std::vector<Subsystem*> m_SystemCache;
	std::unordered_map<StringHash, const TypeInfo*> m_Factories;
	std::unordered_map<StringHash, std::set<const TypeInfo*>> m_TypeTrees;

	int m_SdlInits = 0;
};