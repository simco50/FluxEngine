#include "FluxEngine.h"
#include "Graphics.h"
#include "RenderTarget.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ShaderVariation.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "UI/ImmediateUI.h"
#include "Texture2D.h"
#include "CommandContext.h"

Shader* Graphics::GetShader(const std::string& filePath)
{
	return GetSubsystem<ResourceManager>()->Load<Shader>(filePath + m_ShaderExtension);
}

ShaderVariation* Graphics::GetShader(const std::string& filePath, const ShaderType type, const std::string& defines)
{
	Shader* pShader = GetShader(filePath);
	if (pShader)
	{
		return pShader->GetOrCreateVariation(type, defines);
	}
	return nullptr;
}