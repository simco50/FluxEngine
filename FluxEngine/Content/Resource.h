#pragma once

class InputStream;
class OutputStream;

class Resource : public Object
{
	FLUX_OBJECT(Resource, Object)

public:
	Resource(Context* pContext);
	virtual ~Resource();

	DELETE_COPY(Resource)

	virtual bool Load(InputStream& inputStream) = 0;
	virtual bool Save(OutputStream& outputStream);

	void SetName(const std::string& name) { m_Name = name; }
	void SetMemoryUsage(const unsigned int memory) { m_MemoryUsage = memory; }

	const std::string& GetName() const { return m_Name; }
	unsigned int GetMemoryUsage() const { return m_MemoryUsage; }

	MulticastDelegate<>& OnLoaded() { return m_OnLoadedEvent; }

protected:
	std::string m_Name;
	MulticastDelegate<> m_OnLoadedEvent;
	unsigned int m_MemoryUsage = 0;
};