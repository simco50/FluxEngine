#include "FluxEngine.h"
#include "Material.h"

#include "External/TinyXml/tinyxml2.h"

#include "Core/Graphics.h"
#include "Core/Shader.h"
#include "Core/ShaderVariation.h"
#include "IO/InputStream.h"
#include "Core/Texture2D.h"
#include "Core/Texture3D.h"
#include "Content/Image.h"
#include "Core/TextureCube.h"

Material::Material(Context* pContext) :
	Resource(pContext)
{
	m_pGraphics = pContext->GetSubsystem<Graphics>();
}

Material::~Material()
{
}

bool Material::Load(InputStream& inputStream)
{
	namespace XML = tinyxml2;

	std::vector<unsigned char> buffer;
	if (!inputStream.ReadAllBytes(buffer))
	{
		return false;
	}

	XML::XMLDocument document;
	if (document.Parse((char*)buffer.data(), buffer.size()) != XML::XML_SUCCESS)
	{
		FLUX_LOG(Warning, "[Material::Load] > %s", document.ErrorStr());
		return false;
	}

	XML::XMLElement* pRootNode = document.FirstChildElement();

	const char* pName = pRootNode->Attribute("name");
	if (pName)
	{
		m_Name = pName;
	}

	XML::XMLElement* pCurrentElement = pRootNode->FirstChildElement("Properties");
	if (pCurrentElement)
	{
		ParseProperties(pCurrentElement);
	}
	pCurrentElement = pRootNode->FirstChildElement("Parameters");
	if (pCurrentElement)
	{
		ParseParameters(pCurrentElement);
	}
	pCurrentElement = pRootNode->FirstChildElement("Shaders");
	if (pCurrentElement)
	{
		ParseShaders(pCurrentElement);
	}
	return true;
}

ShaderVariation* Material::GetShader(const ShaderType type) const
{
	return m_ShaderVariations[(unsigned int)type];
}

void Material::SetTexture(TextureSlot slot, Texture* pTexture)
{
	m_Textures[slot] = pTexture;
}

bool Material::ParseShaders(tinyxml2::XMLElement* pElement)
{
	namespace XML = tinyxml2;
	std::array<ShaderVariation*, (size_t)ShaderType::MAX> newShaders = {};
	XML::XMLElement* pShader = pElement->FirstChildElement();
	while (pShader != nullptr)
	{
		std::string shaderType = pShader->Attribute("type");
		ShaderType type;
		if (shaderType == "Vertex")
			type = ShaderType::VertexShader;
		else if (shaderType == "Pixel")
			type = ShaderType::PixelShader;
#ifdef SHADER_GEOMETRY_ENABLE
		else if (shaderType == "Geometry")
			type = ShaderType::GeometryShader;
#endif
#ifdef SHADER_COMPUTE_ENABLE
		else if (shaderType == "Compute")
			type = ShaderType::ComputeShader;
#endif
#ifdef SHADER_TESSELLATION_ENABLE
		else if (shaderType == "Domain")
			type = ShaderType::DomainShader;
		else if (shaderType == "Hull")
			type = ShaderType::HullShader;
#endif
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
		std::string defines;
		if (pAttribute)
		{
			defines = pAttribute;
		}
		std::string source = pShader->Attribute("source");
		ShaderVariation* pShaderVariation = m_pGraphics->GetShader(source, type, defines + m_InternalDefines);
		if (pShaderVariation == nullptr)
		{
			FLUX_LOG(Warning, "[Material::Load] > Shader '%s' for slot '%s' could not be loaded", source.c_str(), shaderType.c_str());
			return false;
		}
		newShaders[(unsigned int)type] = pShaderVariation;
		pShader = pShader->NextSiblingElement();
	}
	m_ShaderVariations.swap(newShaders);
	return true;
}

bool Material::ParseProperties(tinyxml2::XMLElement* pElement)
{
	namespace XML = tinyxml2;
	XML::XMLElement* pProperty = pElement->FirstChildElement();
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
		else if (propertyType == "FillMode")
		{
			std::string value = pProperty->Attribute("value");
			if (value == "Solid")
				m_FillMode = FillMode::SOLID;
			else if (value == "Wireframe")
				m_FillMode = FillMode::WIREFRAME;
			else
				FLUX_LOG(Warning, "[Material::Load()] > %s : Fill mode '%s' is not valid, falling back to default", m_Name.c_str(), value.c_str());
		}
		else if (propertyType == "DepthWrite")
		{
			std::string value = pProperty->Attribute("value");
			if (value == "False")
				m_DepthWrite = false;
			else
				m_DepthWrite = true;
		}
		else
		{
			FLUX_LOG(Warning, "[Material::Load()] > %s : Property with name '%s' is not valid, skipping", m_Name.c_str(), propertyType.c_str());
		}

		pProperty = pProperty->NextSiblingElement();
	}
	return true;
}

bool Material::ParseParameters(tinyxml2::XMLElement* pElement)
{
	namespace XML = tinyxml2;
	XML::XMLElement* pParameter = pElement->FirstChildElement();
	while (pParameter != nullptr)
	{
		std::string parameterType = pParameter->Value();
		if (parameterType == "Texture" || parameterType == "Texture3D" || parameterType == "TextureCube")
		{
			std::string slot = pParameter->Attribute("slot");
			TextureSlot slotType = TextureSlot::MAX;
			if (slot == "Diffuse")
				slotType = TextureSlot::Diffuse;
			else if (slot == "Normal")
				slotType = TextureSlot::Normal;
			else if (slot == "Specular")
				slotType = TextureSlot::Specular;
			else if (slot == "Volume")
				slotType = TextureSlot::Volume;
			else if (slot == "Cube")
				slotType = TextureSlot::Cube;
			else if (slot == "PositionMorph")
				slotType = TextureSlot::PositionMorph;
			else if (slot == "NormalMorph")
				slotType = TextureSlot::NormalMorph;
			else
			{
				FLUX_LOG(Warning, "[Material::Load()] > %s : Slot with name '%s' is not valid", m_Name.c_str(), slot.c_str());
				return false;
			}

			Texture* pTexture;
			if (parameterType == "Texture")
			{
				pTexture = GetSubsystem<ResourceManager>()->Load<Texture2D>(pParameter->Attribute("value"));
			}
			else if(parameterType == "Texture3D")
			{
				pTexture = GetSubsystem<ResourceManager>()->Load<Texture3D>(pParameter->Attribute("value"));
			}
			else if (parameterType == "TextureCube")
			{
				pTexture = GetSubsystem<ResourceManager>()->Load<TextureCube>(pParameter->Attribute("value"));
			}
			else
			{
				FLUX_LOG(Warning, "[Material::Load()] Texture '%s' is unknown", parameterType.c_str());
				return false;
			}
			if (pTexture)
			{
				if (slotType == TextureSlot::Normal && pTexture->GetImage()->GetFormat() == ImageFormat::BC5)
				{
					m_InternalDefines += ",NORMALMAP_BC5";
				}
				m_Textures[slotType] = pTexture;
			}
		}
		else if (parameterType == "Value")
		{
			std::string name = pParameter->Attribute("name");
			std::string value = pParameter->Attribute("value");
			ParseValue(name, value);
		}
		else
		{
			FLUX_LOG(Warning, "[Material::Load()] > %s : Parameter with name '%s' is not valid, skipping", m_Name.c_str(), parameterType.c_str());
		}

		pParameter = pParameter->NextSiblingElement();
	}

	RefreshMemoryUsage();

	return true;
}

//#todo: This is really hacky and unsafe, should find a better way to store arbitrary shader values
void Material::ParseValue(const std::string& name, const std::string& valueString)
{
	AUTOPROFILE_DESC(Material_ParseValue, name);

	std::stringstream stream(valueString);
	std::string stringValue;
	std::vector<int> values;
	while (std::getline(stream, stringValue, ' '))
	{
		if (stringValue.find('.') != std::string::npos)
		{
			float fVal = stof(stringValue);
			values.push_back(*(int*)&fVal);
		}
		else
		{
			values.push_back(stoi(stringValue));
		}
	}
	const size_t byteSize = values.size() * sizeof(float);
	ParameterEntry& entry = m_ParameterCache.GetParameter(name, byteSize);
	entry.SetData(values.data(), byteSize);
}

void Material::RefreshMemoryUsage()
{
	unsigned int memoryUsage = sizeof(Material);
	memoryUsage += (unsigned int)m_ParameterCache.ByteSize();
	memoryUsage += (unsigned int)m_ShaderVariations.size() * sizeof(ShaderVariation);
	memoryUsage += (unsigned int)m_Textures.size() * sizeof(Texture);
	SetMemoryUsage(memoryUsage);
}
