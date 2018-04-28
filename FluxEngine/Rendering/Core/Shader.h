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

	ShaderVariation* GetOrCreateVariation(const ShaderType type, const std::string& defines = std::string(""));

	bool ReloadVariations();

	const std::string& GetSource() { return m_ShaderSource; }
	static std::string GetEntryPoint(const ShaderType type);
	const std::string& GetShaderName() const { return m_ShaderName; }
	const DateTime& GetLastModifiedTimestamp() const { return m_LastModifiedTimestamp; }

private:
	bool ProcessSource(InputStream* pInputStream, std::stringstream& output, std::vector<size_t>& processedIncludes);

	std::string m_ShaderName;
	std::string m_ShaderSource;

	DateTime m_LastModifiedTimestamp;

	std::array<std::map<size_t, std::unique_ptr<ShaderVariation>>, (size_t)ShaderType::MAX> m_ShaderCache;
	void RefreshMemoryUsage();
};