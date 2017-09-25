#include "stdafx.h"
#include "Shader.h"
#include "ShaderVariation.h"

Shader::Shader(ID3D11Device* pDevice) :
	m_pDevice(pDevice)
{

}

Shader::~Shader()
{
}

bool Shader::Load(const string& filePath)
{
	m_FileDir = filePath.substr(0, filePath.rfind('/') + 1);

	ifstream stream;
	stream.open(filePath);
	if (stream.fail())
	{
		FLUX_LOG(WARNING, "Failed to load file: '%s'", filePath.c_str());
		return false;
	}
	string code;
	if (!ProcessSource(stream, code))
		return false;

	m_VertexShaderSource = code;
	m_PixelShaderSource = code;

	CommentFunction(m_VertexShaderSource, "void PS(");
	CommentFunction(m_PixelShaderSource, "void VS(");

	return true;
}

ShaderVariation* Shader::GetVariation(ShaderType type, const vector<string>& defines)
{
	ShaderVariation* variation = new ShaderVariation(this, type);
	variation->SetDefines(defines);
	if (!variation->Create(m_pDevice))
	{
		FLUX_LOG(ERROR, "[Shader::GetVariation()] > Failed to load shader variation");
		return nullptr;
	}
	return variation;
}

const std::string& Shader::GetSource(ShaderType type) const
{
	switch (type)
	{
	case ShaderType::VertexShader:
		return m_VertexShaderSource;
		break;
	case ShaderType::PixelShader:
		return m_PixelShaderSource;
		break;
	default:
		break;
	}
	throw;
}

void Shader::Compile()
{
}

bool Shader::ProcessSource(ifstream& stream, string& output)
{
	string line;
	while (getline(stream, line))
	{
		if (line.substr(0, 8) == "#include")
		{
			string includeFilePath = line.substr(9);
			includeFilePath.erase(includeFilePath.begin());
			includeFilePath.pop_back();
			ifstream newStream(m_FileDir + includeFilePath);
			if (newStream.fail())
				return false;

			if(!ProcessSource(newStream, output))
				return false;
		}
		else
		{
			output += line;
			output += "\n";
		}
	}
	output += "\n";
	stream.close();
	return true;
}

void Shader::CommentFunction(string& input, const string& function)
{
	size_t startPos = input.find(function);
	if (startPos == string::npos)
		return;
	input.insert(startPos, "/*");

	int braceCount = 0;

	for (size_t i = startPos + function.size(); i < input.size(); ++i)
	{
		if (input[i] == '{')
		{
			++braceCount;
			continue;
		}
		if (input[i] == '}')
		{
			--braceCount;
			if (braceCount == 0)
			{
				input.insert(i + 1, "*/");
				break;
			}
			continue;
		}
	}
}
