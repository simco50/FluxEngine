#include "stdafx.h"
#include "Material.h"

#include "External/TinyXml/tinyxml2.h"

#include "Core/Graphics.h"
#include "Core/Shader.h"
#include "Core/ShaderVariation.h"

namespace XML = tinyxml2;

Material::~Material()
{
	for (auto pShader : m_Shaders)
		SafeDelete(pShader.second);
}

std::unique_ptr<Material> Material::Load(std::string filePath, Graphics* pGraphics)
{
	AUTOPROFILE(Material_Load);

	unique_ptr<IFile> pFile = FileSystem::GetFile(filePath);
	if (pFile == nullptr)
		return nullptr;
	if (!pFile->Open(FileMode::Read, ContentType::Text))
		return nullptr;
	vector<char> buffer;
	if (!pFile->ReadAllBytes(buffer))
		return nullptr;
	unique_ptr<Material> pMaterial = make_unique<Material>(pGraphics);

	XML::XMLDocument document;
	if (document.Parse(buffer.data(), buffer.size()) != XML::XML_SUCCESS)
	{
		FLUX_LOG(ERROR, "[Material::Load] > %s", document.ErrorStr());
		return nullptr;
	}
	
	XML::XMLElement* pRootNode = document.FirstChildElement();
	pMaterial->m_Name = pRootNode->Attribute("name");

	//Load the shader data
	XML::XMLElement* pShaders = pRootNode->FirstChildElement("Shaders");
	if (pShaders == nullptr)
		return nullptr;

	XML::XMLElement* pShader = pShaders->FirstChildElement();
	while (pShader != nullptr)
	{
		string shaderType = pShader->Attribute("type");
		ShaderType type = ShaderType::NONE;
		if (shaderType == "Vertex")
			type = ShaderType::VertexShader;
		else if (shaderType == "Pixel")
			type = ShaderType::PixelShader;
		else if (shaderType == "Geometry")
			type = ShaderType::GeometryShader;
		else if (shaderType == "Compute")
			type = ShaderType::ComputeShader;
		else
			return nullptr;

		checkf(pMaterial->m_ShaderVariations[(unsigned int)type] == nullptr, "Shader for slot already defined");

		string source = pShader->Attribute("source");
		auto pIt = pMaterial->m_Shaders.find(source);
		if (pIt == pMaterial->m_Shaders.end())
		{
			pMaterial->m_Shaders[source] = new Shader(pGraphics);
			pMaterial->m_Shaders[source]->Load(source);
		}

		const char* pAttribute = pShader->Attribute("defines");
		string defines = "";
		if (pAttribute)
			defines = pAttribute;

		pMaterial->m_ShaderVariations[(unsigned int)type] = pMaterial->m_Shaders[source]->GetVariation(type, defines);
		pShader = pShader->NextSiblingElement();
	}


	//Load the Parameter data
	XML::XMLElement* pParameters = pRootNode->FirstChildElement("Parameters");
	if (pParameters != nullptr)
	{
		XML::XMLElement* pParameter = pParameters->FirstChildElement();
		while (pParameter != nullptr)
		{
			string parameterType = pParameter->Value();
			if (parameterType == "Texture")
			{
				string slot = pParameter->Attribute("slot");
				TextureSlot slotType = TextureSlot::MAX;
				if (slot == "Diffuse")
					slotType = TextureSlot::Diffuse;
				else if (slot == "Normal")
					slotType = TextureSlot::Normal;
				else if (stoi(slot) < (int)TextureSlot::MAX)
					slotType = (TextureSlot)stoi(slot);
				else 
					return nullptr;
					
				auto pIt = pMaterial->m_Textures.find(slotType);
				checkf(pIt == pMaterial->m_Textures.end(), "Texture for slot already defined");

				//pMaterial->m_Textures[slotType] = ResourceManager::Load<Texture>(pParameter->Attribute("value"));
			}
			else if (parameterType == "Value")
			{
				///
			}

			pParameter = pParameter->NextSiblingElement();
		}
	}

	//Load the properties
	XML::XMLElement* pProperties = pRootNode->FirstChildElement("Properties");
	if (pProperties != nullptr)
	{
		XML::XMLElement* pProperty = pProperties->FirstChildElement();
		while (pProperty != nullptr)
		{
			string propertyType = pProperty->Value();
			if (propertyType == "CullMode")
			{
				string value = pProperty->Attribute("value");
				if (value == "Back")
					pMaterial->m_CullMode = CullMode::BACK;
				else if (value == "Front")
					pMaterial->m_CullMode = CullMode::FRONT;
				else
					pMaterial->m_CullMode = CullMode::NONE;
			}
			else if (propertyType == "Blending")
			{
				pMaterial->m_Blending = pProperty->BoolAttribute("value");
			}
			else
				return nullptr;

			pProperty = pProperty->NextSiblingElement();
		}
	}
	
	return std::move(pMaterial);
}