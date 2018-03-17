#include "FluxEngine.h"
#include "Material.h"

#include "External/TinyXml/tinyxml2.h"

#include "Core/Graphics.h"
#include "Core/Shader.h"
#include "Core/ShaderVariation.h"
#include "IO/InputStream.h"
#include "Core/Texture2D.h"
#include "Core/Texture3D.h"

Material::Material(Context* pContext) :
	Resource(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

Material::~Material()
{
	for (auto& pIt : m_Parameters)
	{
		if (pIt.second.pData)
			delete[] pIt.second.pData;
	}
}

bool Material::Load(InputStream& inputStream)
{
	namespace XML = tinyxml2;

	std::vector<unsigned char> buffer;
	if (!inputStream.ReadAllBytes(buffer))
		return false;

	XML::XMLDocument document;
	if (document.Parse((char*)buffer.data(), buffer.size()) != XML::XML_SUCCESS)
	{
		FLUX_LOG(Warning, "[Material::Load] > %s", document.ErrorStr());
		return false;
	}

	XML::XMLElement* pRootNode = document.FirstChildElement();

	const char* pName = pRootNode->Attribute("name");
	if (pName)
		m_Name = pName;

	XML::XMLElement* pCurrentElement = pRootNode->FirstChildElement();
	while (pCurrentElement != nullptr)
	{
		if (strcmp(pCurrentElement->Value(), "Shaders") == 0)
		{
			std::array<ShaderVariation*, (size_t)ShaderType::MAX> newShaders = {};
			XML::XMLElement* pShader = pCurrentElement->FirstChildElement();
			while (pShader != nullptr)
			{
				std::string shaderType = pShader->Attribute("type");
				ShaderType type;
				if (shaderType == "Vertex")
					type = ShaderType::VertexShader;
				else if (shaderType == "Pixel")
					type = ShaderType::PixelShader;
				else if (shaderType == "Geometry")
					type = ShaderType::GeometryShader;
				else if (shaderType == "Compute")
					type = ShaderType::ComputeShader;
				else
				{
					FLUX_LOG(Warning, "[Material::Load()] > %s : Shader type '%s' is invalid", m_Name.c_str(), shaderType.c_str());
					return false;
				}

				if (newShaders[(unsigned int)type] != nullptr)
				{
					FLUX_LOG(Warning, "[Material::Load] > Shader for slot '%s' already defined", shaderType.c_str());
					return false;
				}

				const char* pAttribute = pShader->Attribute("defines");
				std::string defines = "";
				if (pAttribute)
					defines = pAttribute;
				std::string source = pShader->Attribute("source");
				ShaderVariation* pShaderVariation = m_pGraphics->GetShader(source, type, defines);
				if (pShaderVariation == nullptr)
				{
					FLUX_LOG(Warning, "[Material::Load] > Shader '%s' for slot '%s' could not be loaded", source.c_str(), shaderType.c_str());
					return false;
				}
				newShaders[(unsigned int)type] = pShaderVariation;
				pShader = pShader->NextSiblingElement();
			}
			m_ShaderVariations.swap(newShaders);
		}
		else if (strcmp(pCurrentElement->Value(), "Properties") == 0)
		{
			XML::XMLElement* pProperty = pCurrentElement->FirstChildElement();
			while (pProperty != nullptr)
			{
				std::string propertyType = pProperty->Value();
				if (propertyType == "CullMode")
				{
					std::string value = pProperty->Attribute("value");
					if (value == "Back")
						m_CullMode = CullMode::BACK;
					else if (value == "Front")
						m_CullMode = CullMode::FRONT;
					else if (value == "None")
						m_CullMode = CullMode::NONE;
					else
						FLUX_LOG(Warning, "[Material::Load()] > %s : Cull mode '%s' is not valid, falling back to default", m_Name.c_str(), value.c_str());
				}
				else if (propertyType == "BlendMode")
				{
					std::string value = pProperty->Attribute("value");
					if (value == "Replace")
						m_BlendMode = BlendMode::REPLACE;
					else if (value == "Alpha")
						m_BlendMode = BlendMode::ALPHA;
					else if (value == "Add")
						m_BlendMode = BlendMode::ADD;
					else if (value == "AddAlpha")
						m_BlendMode = BlendMode::ADDALPHA;
					else
						FLUX_LOG(Warning, "[Material::Load()] > %s : Blend mode '%s' is not valid, falling back to default", m_Name.c_str(), value.c_str());
				}
				else
					FLUX_LOG(Warning, "[Material::Load()] > %s : Property with name '%s' is not valid, skipping", m_Name.c_str(), propertyType.c_str());

				pProperty = pProperty->NextSiblingElement();
			}
		}
		else if (strcmp(pCurrentElement->Value(), "Parameters") == 0)
		{
			XML::XMLElement* pParameter = pCurrentElement->FirstChildElement();
			while (pParameter != nullptr)
			{
				std::string parameterType = pParameter->Value();
				if (parameterType == "Texture" || parameterType == "Texture3D")
				{
					std::string slot = pParameter->Attribute("slot");
					TextureSlot slotType = TextureSlot::MAX;
					if (slot == "Diffuse")
						slotType = TextureSlot::Diffuse;
					else if (slot == "Normal")
						slotType = TextureSlot::Normal;
					else if (slot == "Volume")
						slotType = TextureSlot::Volume;
					else
					{
						FLUX_LOG(Warning, "[Material::Load()] > %s : Slot with name '%s' is not valid", m_Name.c_str(), slot.c_str());
						return false;
					}

					Texture* pTexture = nullptr;
					if (parameterType == "Texture3D")
						pTexture = GetSubsystem<ResourceManager>()->Load<Texture3D>(pParameter->Attribute("value"));
					else
						pTexture = GetSubsystem<ResourceManager>()->Load<Texture2D>(pParameter->Attribute("value"));
					m_Textures[slotType] = pTexture;
				}
				else if (parameterType == "Value")
				{
					std::string name = pParameter->Attribute("name");
					std::string value = pParameter->Attribute("value");
					ParseValue(name, value);
				}
				else
				{
					FLUX_LOG(Warning, "[Material::Load()] > %s : Parameter with name '%' is not valid, skipping", m_Name.c_str(), parameterType.c_str());
				}

				pParameter = pParameter->NextSiblingElement();
			}
		}

		pCurrentElement = pCurrentElement->NextSiblingElement();
	}
	return true;
}

ShaderVariation* Material::GetShader(const ShaderType type) const
{
	return m_ShaderVariations[(unsigned int)type];
}

void Material::SetTexture(const TextureSlot slot, Texture* pTexture)
{
	for (auto& p : m_Textures)
	{
		if (p.first == slot)
		{
			p.second = pTexture;
			return;
		}
	}
	m_Textures[slot] = pTexture;
}

//#todo: This is really hacky and unsafe, should find a better way to store arbitrary shader values
void Material::ParseValue(const std::string& name, const std::string& valueString)
{
	AUTOPROFILE_DESC(Material_ParseValue, name);

	std::stringstream stream(valueString);
	std::string stringValue;
	std::vector<float> values;
	while (std::getline(stream, stringValue, ' '))
	{
		values.push_back(stof(stringValue));
	}
	size_t byteSize = values.size() * sizeof(float);
	auto it = m_Parameters.find(name);
	if (it != m_Parameters.end() && it->second.Size == byteSize)
	{
		memcpy(it->second.pData, values.data(), byteSize);
	}
	else
	{
		void* pPointer = new char[byteSize];
		ParameterEntry e(byteSize, pPointer);
		memcpy(pPointer, values.data(), sizeof(float) * values.size());
		m_Parameters[name] = e;
	}
}