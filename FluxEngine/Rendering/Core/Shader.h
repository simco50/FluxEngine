#pragma once
#include "Content/Resource.h"
#include "GraphicsDefines.h"

class Graphics;
class ShaderVariation;

class Shader : public Resource
{
	FLUX_OBJECT(Shader, Resource)

public:
	Shader(Context* pContext);
	~Shader();

	DELETE_COPY(Shader)

	virtual bool Load(InputStream& inputStream) override;

	ShaderVariation* GetOrCreateVariation(ShaderType type, const std::string& defines = std::string(""));

	bool ReloadVariations();

	const std::string& GetSource() const { return m_ShaderSource; }
	static const char* GetEntryPoint(ShaderType type);
	const std::string& GetName() const { return m_Name; }
	const DateTime& GetLastModifiedTimestamp() const { return m_LastModifiedTimestamp; }

private:
	bool ProcessSource(InputStream& inputStream, std::stringstream& output, std::vector<size_t>& processedIncludes);

	std::string m_Name;
	std::string m_ShaderSource;

	DateTime m_LastModifiedTimestamp;

	std::array<std::map<size_t, std::unique_ptr<ShaderVariation>>, (size_t)ShaderType::MAX> m_ShaderCache;
	void RefreshMemoryUsage();
};