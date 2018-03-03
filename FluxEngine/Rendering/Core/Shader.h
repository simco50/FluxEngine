#pragma once
#include "Content\Resource.h"

enum class ShaderType;

class Graphics;
class ShaderVariation;
class File;

class Shader : public Resource
{
	FLUX_OBJECT(Shader, Resource)

public:
	Shader(Context* pContext);
	~Shader();

	DELETE_COPY(Shader)

	virtual bool Load(InputStream& inputStream) override;
	ShaderVariation* GetVariation(const ShaderType type, const std::string& defines = std::string(""));
	const std::string& GetSource() { return m_ShaderSource; }

	static std::string GetEntryPoint(const ShaderType type);

	const std::string& GetName() const { return m_ShaderName; }

private:
	std::string MakeSearchHash(const ShaderType type, const std::string& defines);
	bool ProcessSource(InputStream* pInputStream, std::stringstream& output);

	std::string m_FileDir;
	std::string m_ShaderName;
	std::string m_ShaderSource;

	std::map<size_t, std::unique_ptr<ShaderVariation>> m_ShaderCache;

	Graphics* m_pGraphics;
};