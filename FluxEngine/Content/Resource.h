#pragma once

class InputStream;
class OutputStream;

class Resource : public Object
{
	FLUX_OBJECT(Resource, Object)
	DELETE_COPY(Resource)

public:
	explicit Resource(Context* pContext);
	virtual ~Resource();

	virtual bool Load(InputStream& inputStream) = 0;
	virtual bool Save(OutputStream& outputStream);

	void SetFilePath(const std::string& name) { m_FilePath = name; }
	void SetMemoryUsage(const unsigned int memory) { m_MemoryUsage = memory; }

	const std::string& GetFilePath() const { return m_FilePath; }
	unsigned int GetMemoryUsage() const { return m_MemoryUsage; }

protected:
	std::string m_FilePath;
	unsigned int m_MemoryUsage = 0;
};